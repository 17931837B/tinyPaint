#ifndef UNDO_SYSTEM_HPP
#define UNDO_SYSTEM_HPP

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <fstream>
#include <stdexcept>

// 画像差分データ構造をクラスに書き換え
#include "ImageDiff.hpp"
// class ImageDiff {
// public:
//     // コンストラクタ
//     ImageDiff() : minX(0), minY(0), maxX(0), maxY(0) {}

//     // RLE圧縮関数
//     static std::vector<unsigned char> compressRLE(const unsigned char* data, int width, int height, int startX, int startY, int endX, int endY);
//     static void decompressRLE(const std::vector<unsigned char>& compressed, unsigned char* data, int width, int height, int startX, int startY, int endX, int endY);

//     // シリアライゼーション
//     void serialize(std::ofstream& file) const;
//     void deserialize(std::ifstream& file);
//     size_t getSerializedSize() const;

//     // ゲッター関数
//     int getMinX() const { return minX; }
//     int getMinY() const { return minY; }
//     int getMaxX() const { return maxX; }
//     int getMaxY() const { return maxY; }
//     const std::vector<unsigned char>& getBeforeData() const { return beforeData; }
//     const std::vector<unsigned char>& getAfterData() const { return afterData; }
    
//     // セッター関数 (UndoSystemからのみ設定できるようにする)
//     void setBoundingBox(int minX, int minY, int maxX, int maxY) {
//         this->minX = minX;
//         this->minY = minY;
//         this->maxX = maxX;
//         this->maxY = maxY;
//     }
//     void setBeforeData(const std::vector<unsigned char>& data) { beforeData = data; }
//     void setAfterData(const std::vector<unsigned char>& data) { afterData = data; }

// private:
//     int minX, minY, maxX, maxY; // バウンディングボックス
//     std::vector<unsigned char> beforeData; // RLE圧縮された前の状態
//     std::vector<unsigned char> afterData;  // RLE圧縮された後の状態
// };

// アンドゥスタックエントリをクラスに書き換え
class UndoStackEntry {
public:
    // コンストラクタ
    UndoStackEntry() : fileOffset(0), dataSize(0), isValid(false) {}
    UndoStackEntry(long offset, size_t size) : fileOffset(offset), dataSize(size), isValid(true) {}

    // ゲッター関数
    long getFileOffset() const { return fileOffset; }
    size_t getDataSize() const { return dataSize; }
    bool getIsValid() const { return isValid; }

    // セッター関数
    void setIsValid(bool valid) { isValid = valid; }
    void setFileOffset(long offset) { fileOffset = offset; }
    void setDataSize(size_t size) { dataSize = size; }

private:
    long fileOffset;    // ファイル内でのオフセット
    size_t dataSize;    // データのサイズ
    bool isValid;       // エントリが有効かどうか
};

// バックグラウンド処理用のタスクをクラスに書き換え
class UndoTask {
public:
    enum Type { SAVE_DIFF, APPLY_UNDO, APPLY_REDO };
    Type type;
    ImageDiff diff;
    int stackIndex;
    
    UndoTask(Type t) : type(t), stackIndex(-1) {}
    UndoTask(Type t, const ImageDiff& d, int idx = -1) : type(t), diff(d), stackIndex(idx) {}
};

class UndoSystem {
private:
    static const int MAX_UNDO_LEVELS = 50;
    static const std::string UNDO_FILE_NAME;
    
    std::vector<UndoStackEntry> undoStack;
    int currentPosition;
    std::string undoFileName;
    std::ofstream undoFile;
    
    // バックグラウンド処理用
    std::thread workerThread;
    std::queue<UndoTask> taskQueue;
    std::mutex queueMutex;
    std::condition_variable taskCondition;
    std::atomic<bool> shouldStop;
    
    // 現在のブラシストローク追跡用
    bool isStrokeActive;
    unsigned char* beforeStrokeData;
    int imageWidth, imageHeight;
    int strokeMinX, strokeMinY, strokeMaxX, strokeMaxY;
    
    void workerFunction();
    void processTask(const UndoTask& task);
    void saveDiffToFile(const ImageDiff& diff, long& offset, size_t& size);
    ImageDiff loadDiffFromFile(long offset, size_t size);
    void updateBoundingBox(float x, float y, float radius);
    
public:
    UndoSystem();
    ~UndoSystem();
    
    bool initialize(int width, int height);
    void cleanup();
    
    // ブラシストローク管理
    void beginStroke();
    void updateStroke(float x, float y, float radius);
    void endStroke();
    
    // アンドゥ・リドゥ操作
    bool canUndo() const;
    bool canRedo() const;
    void undo();
    void redo();
    
    // 統計情報
    int getUndoCount() const;
    int getRedoCount() const;
    size_t getFileSize() const;
};

// グローバルなアンドゥシステムインスタンス
extern UndoSystem* g_undoSystem;

// 初期化・クリーンアップ関数
bool initializeUndoSystem(int width, int height);
void cleanupUndoSystem();

#endif