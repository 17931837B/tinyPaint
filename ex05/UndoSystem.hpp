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
#include "UndoTask.hpp"

#define UNDO_FILE_NAME ".undo_stack.dat"
#define MAX_UNDO_LEVELS 200

class UndoSystem
{
	private:
		std::vector<UndoStackEntry>	_undoStack;
		int							_currentPosition;
		std::string					_undoFileName;
		std::ofstream				_undoFile;
		
		// バックグラウンド処理用
		std::thread					_workerThread;
		std::queue<UndoTask>		_taskQueue;
		std::mutex					_queueMutex;
		std::condition_variable		_taskCondition;
		std::atomic<bool>			_shouldStop; // 複数のスレッドが同時に読み書きしても安全なように設計する型
		
		// 現在のブラシストローク追跡用
		bool						_isStrokeActive;
		unsigned char*				_beforeStrokeData;
		int							_imageWidth;
		int							_imageHeight;
		int							_strokeMinX;
		int							_strokeMinY;
		int							_strokeMaxX;
		int							_strokeMaxY;
		
		void		workerFunction();
		void		processTask(const UndoTask& task);
		void		saveDiffToFile(const ImageDiff& diff, long& offset, size_t& size);
		ImageDiff	loadDiffFromFile(long offset, size_t size);
		void		updateBoundingBox(float x, float y, float radius);
		
	public:
		UndoSystem();
		~UndoSystem();
		
		bool		initialize(int width, int height);
		void		cleanup();
		
		// ブラシストローク管理
		void		beginStroke();
		void		updateStroke(float x, float y, float radius);
		void		endStroke();
		
		// アンドゥ・リドゥ操作
		bool		canUndo() const;
		bool		canRedo() const;
		void		undo();
		void		redo();
		
		// 統計情報
		int			getUndoCount() const;
		int			getRedoCount() const;
		size_t		getFileSize() const;
};

// グローバルなアンドゥシステムインスタンス
extern UndoSystem*	g_undoSystem;

// 初期化・クリーンアップ関数
bool	initializeUndoSystem(int width, int height);
void	cleanupUndoSystem();

#endif