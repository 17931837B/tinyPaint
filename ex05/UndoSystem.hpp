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

#include "ImageDiff.hpp"
#include "UndoStackEntry.hpp"

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