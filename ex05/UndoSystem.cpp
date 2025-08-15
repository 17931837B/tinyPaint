#include "UndoSystem.hpp"
#include "paint.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>

UndoSystem*	g_undoSystem = nullptr;

UndoSystem::UndoSystem() : _currentPosition(-1), _shouldStop(false), _isStrokeActive(false), _beforeStrokeData(nullptr), _imageWidth(0), _imageHeight(0)
{
	_undoStack.reserve(MAX_UNDO_LEVELS);
}

UndoSystem::~UndoSystem()
{
	cleanup();
}

bool	UndoSystem::initialize(int width, int height)
{
	_imageWidth = width;
	_imageHeight = height;
	_beforeStrokeData = new unsigned char[width * height * 4];
	// もし既存の.undo_stack.datがあったら削除
	remove(UNDO_FILE_NAME);
	// .undo_stack.datを作成
	_undoFile.open(UNDO_FILE_NAME, std::ios::binary | std::ios::out | std::ios::app);
	if (!_undoFile.is_open())
	{
		std::cerr << "Error: " << UNDO_FILE_NAME << std::endl;
		return (false);
	}
	// スレッド開始
	_shouldStop = false;
	_workerThread = std::thread(&UndoSystem::workerFunction, this);
	return (true);
}

// バックグラウンドのスレッドループ処理
void UndoSystem::workerFunction()
{
	while (!_shouldStop)
	{
		// タスクキューをロック
		std::unique_lock<std::mutex> lock(_queueMutex);
		// タスクが来るかstopシグナルが来るまで待機
		_taskCondition.wait(lock, [this]{return !_taskQueue.empty() || _shouldStop; });
		// _shouldStoでwaitを抜けていたらループを抜ける
		if (_shouldStop)
			break ;
		// キューからタスクを取得
		UndoTask task = _taskQueue.front();
		_taskQueue.pop();
		// ロックを解除
		lock.unlock();
		processTask(task);
	}
}

// undoのシャットダウン
void	UndoSystem::cleanup()
{
	// スレッドを停止
	_shouldStop = true;
	_taskCondition.notify_all();
	// メインスレッドの実行を待機
	if (_workerThread.joinable())
		_workerThread.join();
	// .undo_stack.datを閉じる
	if (_undoFile.is_open())
		_undoFile.close();
	// バッファの解放
	if (_beforeStrokeData)
	{
		delete[] _beforeStrokeData;
		_beforeStrokeData = nullptr;
	}
	// .undo_stack.datを削除
	remove(UNDO_FILE_NAME);
}

// 書き出し前の状態保存
void	UndoSystem::beginStroke()
{
	// ストローク中ならば何もせず
	if (_isStrokeActive)
		return ;
	_isStrokeActive = true;
	_strokeMinX = _imageWidth;
	_strokeMinY = _imageHeight;
	_strokeMaxX = -1;
	_strokeMaxY = -1;
	// 現在の画像状態を保存
	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	glReadPixels(0, 0, _imageWidth, _imageHeight, GL_RGBA, GL_UNSIGNED_BYTE, _beforeStrokeData);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void	UndoSystem::updateStroke(float x, float y, float radius)
{
	if (!_isStrokeActive)
		return ;
	updateBoundingBox(x, y, radius);
}

// バウディングボックスで変更する範囲を更新
void	UndoSystem::updateBoundingBox(float x, float y, float radius)
{
	int	minX;
	int	minY;
	int	maxX;
	int	maxY;

	minX = (int)(x - radius);
	minY = (int)(y - radius);
	maxX = (int)(x + radius);
	maxY = (int)(y + radius);
	
	_strokeMinX = std::min(_strokeMinX, minX);
	_strokeMinY = std::min(_strokeMinY, minY);
	_strokeMaxX = std::max(_strokeMaxX, maxX);
	_strokeMaxY = std::max(_strokeMaxY, maxY);
}

void	UndoSystem::endStroke()
{
	int							margin;
	unsigned char*				afterStrokeData;
	ImageDiff					diff;
	std::lock_guard<std::mutex>	lock(_queueMutex);
	size_t						i;

	if (!_isStrokeActive)
		return ;
	_isStrokeActive = false;
	
	// バウンディングボックスが更新されたか
	if (_strokeMaxX < _strokeMinX || _strokeMaxY < _strokeMinY)
		return ;
	// マージンを設ける
	margin = 5;
	_strokeMinX = std::max(0, _strokeMinX - margin);
	_strokeMinY = std::max(0, _strokeMinY - margin);
	_strokeMaxX = std::min(_imageWidth - 1, _strokeMaxX + margin);
	_strokeMaxY = std::min(_imageHeight - 1, _strokeMaxY + margin);
	// 現在の画像状態を読み取り
	afterStrokeData = new unsigned char[_imageWidth * _imageHeight * 4];
	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	glReadPixels(0, 0, _imageWidth, _imageHeight, GL_RGBA, GL_UNSIGNED_BYTE, afterStrokeData);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// RLE圧縮を用いて差分を作成
	diff.setBeforeData(ImageDiff::compressRLE(_beforeStrokeData, _imageWidth, _imageHeight, _strokeMinX, _strokeMinY, _strokeMaxX, _strokeMaxY));
	diff.setAfterData(ImageDiff::compressRLE(afterStrokeData, _imageWidth, _imageHeight, _strokeMinX, _strokeMinY, _strokeMaxX, _strokeMaxY));
	// バウンディングボックスをdiffに設定
	diff.setBoundingBox(_strokeMinX, _strokeMinY, _strokeMaxX, _strokeMaxY);
	delete[] afterStrokeData;
	// 履歴の枝分かれ対策
	i = _currentPosition + 1;
	while (i < _undoStack.size())
	{
		_undoStack[i].setIsValid(false);
		i++;
	}
	_undoStack.erase(_undoStack.begin() + _currentPosition + 1, _undoStack.end());
	// スタックサイズを念の為制限(安全のため、いらないかも、)
	if (_undoStack.size() >= MAX_UNDO_LEVELS)
	{
		_undoStack.erase(_undoStack.begin());
		_currentPosition--;
	}
	_taskQueue.push(UndoTask(UndoTask::SAVE_DIFF, diff, _undoStack.size()));
	// バックグラウンドスレッドに作業が来たことを通知
	_taskCondition.notify_one();
}

bool	UndoSystem::canUndo() const
{
	return (_currentPosition >= 0);
}

bool	UndoSystem::canRedo() const
{
	// リドゥ履歴がアンドゥスタック内に存在するか。また、リドゥ対象の履歴が有効であるか。
	return (static_cast<size_t>(_currentPosition + 1) < _undoStack.size() && _undoStack[_currentPosition + 1].getIsValid());
}

// undo操作
void	UndoSystem::undo()
{
	UndoStackEntry	entry;
	ImageDiff		diff;
	unsigned char*	currentData;
	int				regionWidth;
	int				regionHeight;
	unsigned char*	regionData;
	int				y;
	int				x;
	int				srcIndex;
	int				dstIndex;

	if (!canUndo())
		return ;
	entry = _undoStack[_currentPosition];
	diff = loadDiffFromFile(entry.getFileOffset(), entry.getDataSize());
	// 現在の画像データを読み取り
	currentData = new unsigned char[_imageWidth * _imageHeight * 4];
	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	glReadPixels(0, 0, _imageWidth, _imageHeight, GL_RGBA, GL_UNSIGNED_BYTE, currentData);
	// beforeデータでストローク前のピクセルデータを、currentDataの該当する領域に上書きして復元
	ImageDiff::decompressRLE(diff.getBeforeData(), currentData, _imageWidth, _imageHeight, diff.getMinX(), diff.getMinY(), diff.getMaxX(), diff.getMaxY());
	// 差分領域のみをテクスチャに更新
	glBindTexture(GL_TEXTURE_2D, texId);
	regionWidth = diff.getMaxX() - diff.getMinX() + 1;
	regionHeight = diff.getMaxY() - diff.getMinY() + 1;
	// 差分領域のデータ抽出
	regionData = new unsigned char[regionWidth * regionHeight * 4];
	y = 0;
	while (y < regionHeight)
	{
		x = 0;
		while (x < regionWidth)
		{
			srcIndex = ((diff.getMinY() + y) * _imageWidth + (diff.getMinX() + x)) * 4;
			dstIndex = (y * regionWidth + x) * 4;
			regionData[dstIndex + 0] = currentData[srcIndex + 0];
			regionData[dstIndex + 1] = currentData[srcIndex + 1];
			regionData[dstIndex + 2] = currentData[srcIndex + 2];
			regionData[dstIndex + 3] = currentData[srcIndex + 3];
			x++;
		}
		y++;
	}
	// 差分領域のみを更新
	glTexSubImage2D(GL_TEXTURE_2D, 0, diff.getMinX(), diff.getMinY(), 
					regionWidth, regionHeight, GL_RGBA, GL_UNSIGNED_BYTE, regionData);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	delete[] currentData;
	delete[] regionData;
	_currentPosition--;
}

// redo操作
void	UndoSystem::redo()
{
	UndoStackEntry	entry;
	ImageDiff		diff;
	unsigned char*	currentData;
	int				regionWidth;
	int				regionHeight;
	unsigned char*	regionData;
	int				y;
	int				x;
	int				srcIndex;
	int				dstIndex;

	if (!canRedo())
		return ;
	// rido対象に移動
	_currentPosition++;
	entry = _undoStack[_currentPosition];
	diff = loadDiffFromFile(entry.getFileOffset(), entry.getDataSize());
	// 現在の画像データを読み取り
	currentData = new unsigned char[_imageWidth * _imageHeight * 4];
	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	glReadPixels(0, 0, _imageWidth, _imageHeight, GL_RGBA, GL_UNSIGNED_BYTE, currentData);
	// afterデータでストローク前のピクセルデータを、currentDataの該当する領域に上書きして復元
	ImageDiff::decompressRLE(diff.getAfterData(), currentData, _imageWidth, _imageHeight, diff.getMinX(), diff.getMinY(), diff.getMaxX(), diff.getMaxY());
	// 差分領域のみをテクスチャに更新
	glBindTexture(GL_TEXTURE_2D, texId);
	regionWidth = diff.getMaxX() - diff.getMinX() + 1;
	regionHeight = diff.getMaxY() - diff.getMinY() + 1;
	// 差分領域のデータ抽出
	regionData = new unsigned char[regionWidth * regionHeight * 4];
	y = 0;
	while (y < regionHeight)
	{
		x = 0;
		while (x < regionWidth)
		{
			srcIndex = ((diff.getMinY() + y) * _imageWidth + (diff.getMinX() + x)) * 4;
			dstIndex = (y * regionWidth + x) * 4;
			regionData[dstIndex + 0] = currentData[srcIndex + 0];
			regionData[dstIndex + 1] = currentData[srcIndex + 1];
			regionData[dstIndex + 2] = currentData[srcIndex + 2];
			regionData[dstIndex + 3] = currentData[srcIndex + 3];
			x++;
		}
		y++;
	}
	// 差分領域のみを更新
	glTexSubImage2D(GL_TEXTURE_2D, 0, diff.getMinX(), diff.getMinY(), 
					regionWidth, regionHeight, GL_RGBA, GL_UNSIGNED_BYTE, regionData);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	delete[] currentData;
	delete[] regionData;
}

ImageDiff	UndoSystem::loadDiffFromFile(long offset, size_t /*size*/)
{
	std::ifstream	file(UNDO_FILE_NAME, std::ios::binary);
	ImageDiff		diff;

	if (!file.is_open())
	{
		std::cerr << "Error: " << UNDO_FILE_NAME << std::endl;
		return ;
	}
	file.seekg(offset);
	diff.deserialize(file);
	return (diff);
}

void	UndoSystem::processTask(const UndoTask& task)
{
	long	offset;
	size_t	size;

	switch (task.getType())
	{
		case UndoTask::SAVE_DIFF:
			saveDiffToFile(task.getDiff(), offset, size);
			{
				std::lock_guard<std::mutex>	lock(_queueMutex);
				UndoStackEntry				entry(offset, size);
				_undoStack.push_back(entry);
				_currentPosition++;
			}
			break;
		default:
			break;
	}
}

void	UndoSystem::saveDiffToFile(const ImageDiff& diff, long& offset, size_t& size)
{
	offset = _undoFile.tellp();
	diff.serialize(_undoFile);
	_undoFile.flush();
	size = diff.getSerializedSize();
}

int	UndoSystem::getUndoCount() const
{
	return (_currentPosition + 1);
}

int	UndoSystem::getRedoCount() const
{
	int		redoCount;
	size_t	i;

	redoCount = 0;
	i = _currentPosition + 1;
	while (i < _undoStack.size())
	{
		if (_undoStack[i].getIsValid())
			redoCount++;
		i++;
	}
	return (redoCount);
}

size_t	UndoSystem::getFileSize() const
{
	std::ifstream	file(UNDO_FILE_NAME, std::ios::binary | std::ios::ate);
	std::streampos	pos;

	if (!file.is_open())
		return (0);
	
	pos = file.tellg();
	return (static_cast<size_t>(pos));
}

// グローバル関数の実装
bool	initializeUndoSystem(int width, int height)
{
	if (g_undoSystem)
	{
		delete g_undoSystem;
	}
	
	g_undoSystem = new UndoSystem();
	return (g_undoSystem->initialize(width, height));
}

void	cleanupUndoSystem()
{
	if (g_undoSystem)
	{
		delete g_undoSystem;
		g_undoSystem = nullptr;
	}
}