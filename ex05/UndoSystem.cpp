#include "UndoSystem.hpp"
#include "paint.hpp"
#include <algorithm>
#include <cstring>

const std::string UndoSystem::UNDO_FILE_NAME = "undo_stack.dat";
UndoSystem* g_undoSystem = nullptr;

// RLE圧縮実装
std::vector<unsigned char> ImageDiff::compressRLE(const unsigned char* data, int width, int /*height*/, int startX, int startY, int endX, int endY) {
    std::vector<unsigned char> compressed;
    
    for (int y = startY; y <= endY; y++) {
        for (int x = startX; x <= endX; ) {
            int pixelIndex = (y * width + x) * 4;
            unsigned char r = data[pixelIndex];
            unsigned char g = data[pixelIndex + 1];
            unsigned char b = data[pixelIndex + 2];
            unsigned char a = data[pixelIndex + 3];
            
            // 同じ色が続く長さを数える
            int runLength = 1;
            int nextX = x + 1;
            while (nextX <= endX && runLength < 255) {
                int nextIndex = (y * width + nextX) * 4;
                if (data[nextIndex] == r && data[nextIndex + 1] == g && 
                    data[nextIndex + 2] == b && data[nextIndex + 3] == a) {
                    runLength++;
                    nextX++;
                } else {
                    break;
                }
            }
            
            // RLEデータを追加
            compressed.push_back(runLength);
            compressed.push_back(r);
            compressed.push_back(g);
            compressed.push_back(b);
            compressed.push_back(a);
            
            x += runLength;
        }
    }
    
    return compressed;
}

void ImageDiff::decompressRLE(const std::vector<unsigned char>& compressed, unsigned char* data, int width, int /*height*/, int startX, int startY, int endX, int endY) {
    size_t compressedIndex = 0;
    int x = startX;
    int y = startY;
    
    while (compressedIndex < compressed.size() && y <= endY) {
        unsigned char runLength = compressed[compressedIndex++];
        unsigned char r = compressed[compressedIndex++];
        unsigned char g = compressed[compressedIndex++];
        unsigned char b = compressed[compressedIndex++];
        unsigned char a = compressed[compressedIndex++];
        
        for (int i = 0; i < runLength && y <= endY; i++) {
            if (x > endX) {
                x = startX;
                y++;
                if (y > endY) break;
            }
            
            int pixelIndex = (y * width + x) * 4;
            data[pixelIndex] = r;
            data[pixelIndex + 1] = g;
            data[pixelIndex + 2] = b;
            data[pixelIndex + 3] = a;
            x++;
        }
    }
}

void ImageDiff::serialize(std::ofstream& file) const {
    file.write(reinterpret_cast<const char*>(&minX), sizeof(minX));
    file.write(reinterpret_cast<const char*>(&minY), sizeof(minY));
    file.write(reinterpret_cast<const char*>(&maxX), sizeof(maxX));
    file.write(reinterpret_cast<const char*>(&maxY), sizeof(maxY));
    
    size_t beforeSize = beforeData.size();
    size_t afterSize = afterData.size();
    file.write(reinterpret_cast<const char*>(&beforeSize), sizeof(beforeSize));
    file.write(reinterpret_cast<const char*>(&afterSize), sizeof(afterSize));
    
    if (beforeSize > 0) {
        file.write(reinterpret_cast<const char*>(beforeData.data()), beforeSize);
    }
    if (afterSize > 0) {
        file.write(reinterpret_cast<const char*>(afterData.data()), afterSize);
    }
}

void ImageDiff::deserialize(std::ifstream& file) {
    file.read(reinterpret_cast<char*>(&minX), sizeof(minX));
    file.read(reinterpret_cast<char*>(&minY), sizeof(minY));
    file.read(reinterpret_cast<char*>(&maxX), sizeof(maxX));
    file.read(reinterpret_cast<char*>(&maxY), sizeof(maxY));
    
    size_t beforeSize, afterSize;
    file.read(reinterpret_cast<char*>(&beforeSize), sizeof(beforeSize));
    file.read(reinterpret_cast<char*>(&afterSize), sizeof(afterSize));
    
    beforeData.resize(beforeSize);
    afterData.resize(afterSize);
    
    if (beforeSize > 0) {
        file.read(reinterpret_cast<char*>(beforeData.data()), beforeSize);
    }
    if (afterSize > 0) {
        file.read(reinterpret_cast<char*>(afterData.data()), afterSize);
    }
}

size_t ImageDiff::getSerializedSize() const {
    return sizeof(minX) + sizeof(minY) + sizeof(maxX) + sizeof(maxY) +
           sizeof(size_t) * 2 + beforeData.size() + afterData.size();
}

UndoSystem::UndoSystem() : currentPosition(-1), shouldStop(false), 
                           isStrokeActive(false), beforeStrokeData(nullptr),
                           imageWidth(0), imageHeight(0) {
    undoStack.reserve(MAX_UNDO_LEVELS);
}

UndoSystem::~UndoSystem() {
    cleanup();
}

bool UndoSystem::initialize(int width, int height) {
    imageWidth = width;
    imageHeight = height;
    
    // 既存のアンドゥファイルを削除
    remove(UNDO_FILE_NAME.c_str());
    
    // アンドゥファイルを作成
    undoFile.open(UNDO_FILE_NAME, std::ios::binary | std::ios::out | std::ios::app);
    if (!undoFile.is_open()) {
        std::cerr << "Failed to create undo file: " << UNDO_FILE_NAME << std::endl;
        return false;
    }
    
    // バックグラウンドワーカースレッドを開始
    shouldStop = false;
    workerThread = std::thread(&UndoSystem::workerFunction, this);
    
    // ストローク用バッファを確保
    beforeStrokeData = new unsigned char[width * height * 4];
    
    std::cout << "Undo system initialized. Max levels: " << MAX_UNDO_LEVELS << std::endl;
    return true;
}

void UndoSystem::cleanup() {
    // ワーカースレッドを停止
    shouldStop = true;
    taskCondition.notify_all();
    if (workerThread.joinable()) {
        workerThread.join();
    }
    
    // ファイルを閉じる
    if (undoFile.is_open()) {
        undoFile.close();
    }
    
    // バッファを解放
    if (beforeStrokeData) {
        delete[] beforeStrokeData;
        beforeStrokeData = nullptr;
    }
    
    // アンドゥファイルを削除
    remove(UNDO_FILE_NAME.c_str());
    
    std::cout << "Undo system cleaned up." << std::endl;
}

void UndoSystem::beginStroke() {
    if (isStrokeActive) return;
    
    isStrokeActive = true;
    strokeMinX = imageWidth;
    strokeMinY = imageHeight;
    strokeMaxX = -1;
    strokeMaxY = -1;
    
    // 現在の画像状態を保存
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);
    glReadPixels(0, 0, imageWidth, imageHeight, GL_RGBA, GL_UNSIGNED_BYTE, beforeStrokeData);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void UndoSystem::updateStroke(float x, float y, float radius) {
    if (!isStrokeActive) return;
    
    updateBoundingBox(x, y, radius);
}

void UndoSystem::endStroke() {
    if (!isStrokeActive) return;
    
    isStrokeActive = false;
    
    // バウンディングボックスが有効かチェック
    if (strokeMaxX < strokeMinX || strokeMaxY < strokeMinY) {
        return; // 何も描画されていない
    }
    
    // マージンを追加
    int margin = 5;
    strokeMinX = std::max(0, strokeMinX - margin);
    strokeMinY = std::max(0, strokeMinY - margin);
    strokeMaxX = std::min(imageWidth - 1, strokeMaxX + margin);
    strokeMaxY = std::min(imageHeight - 1, strokeMaxY + margin);
    
    // 現在の画像状態を読み取り
    unsigned char* afterStrokeData = new unsigned char[imageWidth * imageHeight * 4];
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);
    glReadPixels(0, 0, imageWidth, imageHeight, GL_RGBA, GL_UNSIGNED_BYTE, afterStrokeData);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // 差分を作成
    ImageDiff diff;
    diff.minX = strokeMinX;
    diff.minY = strokeMinY;
    diff.maxX = strokeMaxX;
    diff.maxY = strokeMaxY;
    
    // RLE圧縮
    diff.beforeData = ImageDiff::compressRLE(beforeStrokeData, imageWidth, imageHeight, 
                                           strokeMinX, strokeMinY, strokeMaxX, strokeMaxY);
    diff.afterData = ImageDiff::compressRLE(afterStrokeData, imageWidth, imageHeight,
                                          strokeMinX, strokeMinY, strokeMaxX, strokeMaxY);
    
    delete[] afterStrokeData;
    
    // バックグラウンドタスクに追加
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        
        // 現在位置より後のエントリを無効化
        for (size_t i = currentPosition + 1; i < undoStack.size(); i++) {
            undoStack[i].isValid = false;
        }
        undoStack.erase(undoStack.begin() + currentPosition + 1, undoStack.end());
        
        // スタックサイズ制限
        if (undoStack.size() >= MAX_UNDO_LEVELS) {
            undoStack.erase(undoStack.begin());
            currentPosition--;
        }
        
        taskQueue.push(UndoTask(UndoTask::SAVE_DIFF, diff, undoStack.size()));
    }
    taskCondition.notify_one();
}

void UndoSystem::updateBoundingBox(float x, float y, float radius) {
    int minX = (int)(x - radius);
    int minY = (int)(y - radius);
    int maxX = (int)(x + radius);
    int maxY = (int)(y + radius);
    
    strokeMinX = std::min(strokeMinX, minX);
    strokeMinY = std::min(strokeMinY, minY);
    strokeMaxX = std::max(strokeMaxX, maxX);
    strokeMaxY = std::max(strokeMaxY, maxY);
}

bool UndoSystem::canUndo() const {
    return currentPosition >= 0;
}

bool UndoSystem::canRedo() const {
    return static_cast<size_t>(currentPosition + 1) < undoStack.size() && undoStack[currentPosition + 1].isValid;
}

void UndoSystem::undo() {
    if (!canUndo()) return;
    
    UndoStackEntry entry = undoStack[currentPosition];
    ImageDiff diff = loadDiffFromFile(entry.fileOffset, entry.dataSize);
    
    // 現在の画像データを読み取り
    unsigned char* currentData = new unsigned char[imageWidth * imageHeight * 4];
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);
    glReadPixels(0, 0, imageWidth, imageHeight, GL_RGBA, GL_UNSIGNED_BYTE, currentData);
    
    // beforeデータで復元（差分領域のみ）
    ImageDiff::decompressRLE(diff.beforeData, currentData, imageWidth, imageHeight,
                           diff.minX, diff.minY, diff.maxX, diff.maxY);
    
    // 差分領域のみをテクスチャに更新
    glBindTexture(GL_TEXTURE_2D, texId);
    int regionWidth = diff.maxX - diff.minX + 1;
    int regionHeight = diff.maxY - diff.minY + 1;
    
    // 差分領域のデータを抽出
    unsigned char* regionData = new unsigned char[regionWidth * regionHeight * 4];
    for (int y = 0; y < regionHeight; y++) {
        for (int x = 0; x < regionWidth; x++) {
            int srcIndex = ((diff.minY + y) * imageWidth + (diff.minX + x)) * 4;
            int dstIndex = (y * regionWidth + x) * 4;
            regionData[dstIndex + 0] = currentData[srcIndex + 0];
            regionData[dstIndex + 1] = currentData[srcIndex + 1];
            regionData[dstIndex + 2] = currentData[srcIndex + 2];
            regionData[dstIndex + 3] = currentData[srcIndex + 3];
        }
    }
    
    // 差分領域のみを更新
    glTexSubImage2D(GL_TEXTURE_2D, 0, diff.minX, diff.minY, 
                    regionWidth, regionHeight, GL_RGBA, GL_UNSIGNED_BYTE, regionData);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    delete[] currentData;
    delete[] regionData;
    
    currentPosition--;
    std::cout << "Undo applied. Position: " << currentPosition << std::endl;
}

void UndoSystem::redo() {
    if (!canRedo()) return;
    
    currentPosition++;
    UndoStackEntry entry = undoStack[currentPosition];
    ImageDiff diff = loadDiffFromFile(entry.fileOffset, entry.dataSize);
    
    // 現在の画像データを読み取り
    unsigned char* currentData = new unsigned char[imageWidth * imageHeight * 4];
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);
    glReadPixels(0, 0, imageWidth, imageHeight, GL_RGBA, GL_UNSIGNED_BYTE, currentData);
    
    // afterデータで復元（差分領域のみ）
    ImageDiff::decompressRLE(diff.afterData, currentData, imageWidth, imageHeight,
                           diff.minX, diff.minY, diff.maxX, diff.maxY);
    
    // 差分領域のみをテクスチャに更新
    glBindTexture(GL_TEXTURE_2D, texId);
    int regionWidth = diff.maxX - diff.minX + 1;
    int regionHeight = diff.maxY - diff.minY + 1;
    
    // 差分領域のデータを抽出
    unsigned char* regionData = new unsigned char[regionWidth * regionHeight * 4];
    for (int y = 0; y < regionHeight; y++) {
        for (int x = 0; x < regionWidth; x++) {
            int srcIndex = ((diff.minY + y) * imageWidth + (diff.minX + x)) * 4;
            int dstIndex = (y * regionWidth + x) * 4;
            regionData[dstIndex + 0] = currentData[srcIndex + 0];
            regionData[dstIndex + 1] = currentData[srcIndex + 1];
            regionData[dstIndex + 2] = currentData[srcIndex + 2];
            regionData[dstIndex + 3] = currentData[srcIndex + 3];
        }
    }
    
    // 差分領域のみを更新
    glTexSubImage2D(GL_TEXTURE_2D, 0, diff.minX, diff.minY, 
                    regionWidth, regionHeight, GL_RGBA, GL_UNSIGNED_BYTE, regionData);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    delete[] currentData;
    delete[] regionData;
    
    std::cout << "Redo applied. Position: " << currentPosition << std::endl;
}

void UndoSystem::workerFunction() {
    while (!shouldStop) {
        std::unique_lock<std::mutex> lock(queueMutex);
        taskCondition.wait(lock, [this] { return !taskQueue.empty() || shouldStop; });
        
        if (shouldStop) break;
        
        UndoTask task = taskQueue.front();
        taskQueue.pop();
        lock.unlock();
        
        processTask(task);
    }
}

void UndoSystem::processTask(const UndoTask& task) {
    switch (task.type) {
        case UndoTask::SAVE_DIFF: {
            long offset;
            size_t size;
            saveDiffToFile(task.diff, offset, size);
            
            std::lock_guard<std::mutex> lock(queueMutex);
            undoStack.push_back(UndoStackEntry(offset, size));
            currentPosition++;
            break;
        }
        default:
            break;
    }
}

void UndoSystem::saveDiffToFile(const ImageDiff& diff, long& offset, size_t& size) {
    offset = undoFile.tellp();
    diff.serialize(undoFile);
    undoFile.flush();
    size = diff.getSerializedSize();
}

ImageDiff UndoSystem::loadDiffFromFile(long offset, size_t /*size*/) {
    std::ifstream file(UNDO_FILE_NAME, std::ios::binary);
    file.seekg(offset);
    
    ImageDiff diff;
    diff.deserialize(file);
    
    return diff;
}

int UndoSystem::getUndoCount() const {
    return currentPosition + 1;
}

int UndoSystem::getRedoCount() const {
    int redoCount = 0;
    for (size_t i = currentPosition + 1; i < undoStack.size(); i++) {
        if (undoStack[i].isValid) redoCount++;
    }
    return redoCount;
}

size_t UndoSystem::getFileSize() const {
    std::ifstream file(UNDO_FILE_NAME, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return 0;
    
    std::streampos pos = file.tellg();
    return static_cast<size_t>(pos);
}

// グローバル関数の実装
bool initializeUndoSystem(int width, int height) {
    if (g_undoSystem) {
        delete g_undoSystem;
    }
    
    g_undoSystem = new UndoSystem();
    return g_undoSystem->initialize(width, height);
}

void cleanupUndoSystem() {
    if (g_undoSystem) {
        delete g_undoSystem;
        g_undoSystem = nullptr;
    }
}