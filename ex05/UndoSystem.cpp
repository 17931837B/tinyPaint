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
	
	// 既存のアンドゥファイルを削除
	remove(UNDO_FILE_NAME);
	
	// アンドゥファイルを作成
	_undoFile.open(UNDO_FILE_NAME, std::ios::binary | std::ios::out | std::ios::app);
	if (!_undoFile.is_open())
	{
		std::cerr << "Failed to create undo file: " << UNDO_FILE_NAME << std::endl;
		return (false);
	}
	
	// バックグラウンドワーカースレッドを開始
	_shouldStop = false;
	_workerThread = std::thread(&UndoSystem::workerFunction, this);
	
	// ストローク用バッファを確保
	_beforeStrokeData = new unsigned char[width * height * 4];
	
	std::cout << "Undo system initialized. Max levels: " << MAX_UNDO_LEVELS << std::endl;
	return (true);
}

void	UndoSystem::cleanup()
{
	// ワーカースレッドを停止
	_shouldStop = true;
	_taskCondition.notify_all();
	if (_workerThread.joinable())
	{
		_workerThread.join();
	}
	
	// ファイルを閉じる
	if (_undoFile.is_open())
	{
		_undoFile.close();
	}
	
	// バッファを解放
	if (_beforeStrokeData)
	{
		delete[] _beforeStrokeData;
		_beforeStrokeData = nullptr;
	}
	
	// アンドゥファイルを削除
	remove(UNDO_FILE_NAME);
	
	std::cout << "Undo system cleaned up." << std::endl;
}

void	UndoSystem::beginStroke()
{
	if (_isStrokeActive)
		return;
	
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
		return;
	
	updateBoundingBox(x, y, radius);
}

void	UndoSystem::endStroke()
{
	int							margin;
	unsigned char*				afterStrokeData;
	ImageDiff					diff;
	std::lock_guard<std::mutex>	lock(_queueMutex);
	size_t						i;

	if (!_isStrokeActive)
		return;
	
	_isStrokeActive = false;
	
	// バウンディングボックスが有効かチェック
	if (_strokeMaxX < _strokeMinX || _strokeMaxY < _strokeMinY)
	{
		return; // 何も描画されていない
	}
	
	// マージンを追加
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
	
	// 差分を作成
	// RLE圧縮
	diff.setBeforeData(ImageDiff::compressRLE(_beforeStrokeData, _imageWidth, _imageHeight, 
											_strokeMinX, _strokeMinY, _strokeMaxX, _strokeMaxY));
	diff.setAfterData(ImageDiff::compressRLE(afterStrokeData, _imageWidth, _imageHeight,
										   _strokeMinX, _strokeMinY, _strokeMaxX, _strokeMaxY));
	
	// バウンディングボックスをdiffに設定
	diff.setBoundingBox(_strokeMinX, _strokeMinY, _strokeMaxX, _strokeMaxY);

	delete[] afterStrokeData;
	
	// バックグラウンドタスクに追加
	// 現在位置より後のエントリを無効化
	i = _currentPosition + 1;
	while (i < _undoStack.size())
	{
		_undoStack[i].setIsValid(false);
		i++;
	}
	_undoStack.erase(_undoStack.begin() + _currentPosition + 1, _undoStack.end());
	
	// スタックサイズ制限
	if (_undoStack.size() >= MAX_UNDO_LEVELS)
	{
		_undoStack.erase(_undoStack.begin());
		_currentPosition--;
	}
	
	_taskQueue.push(UndoTask(UndoTask::SAVE_DIFF, diff, _undoStack.size()));
	
	_taskCondition.notify_one();
}

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

bool	UndoSystem::canUndo() const
{
	return (_currentPosition >= 0);
}

bool	UndoSystem::canRedo() const
{
	return (static_cast<size_t>(_currentPosition + 1) < _undoStack.size() && _undoStack[_currentPosition + 1].getIsValid());
}

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
		return;
	
	entry = _undoStack[_currentPosition];
	diff = loadDiffFromFile(entry.getFileOffset(), entry.getDataSize());
	
	// 現在の画像データを読み取り
	currentData = new unsigned char[_imageWidth * _imageHeight * 4];
	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	glReadPixels(0, 0, _imageWidth, _imageHeight, GL_RGBA, GL_UNSIGNED_BYTE, currentData);
	
	// beforeデータで復元（差分領域のみ）
	ImageDiff::decompressRLE(diff.getBeforeData(), currentData, _imageWidth, _imageHeight,
						   diff.getMinX(), diff.getMinY(), diff.getMaxX(), diff.getMaxY());
	
	// 差分領域のみをテクスチャに更新
	glBindTexture(GL_TEXTURE_2D, texId);
	regionWidth = diff.getMaxX() - diff.getMinX() + 1;
	regionHeight = diff.getMaxY() - diff.getMinY() + 1;
	
	// 差分領域のデータを抽出
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
	std::cout << "Undo applied. Position: " << _currentPosition << std::endl;
}

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
		return;
	
	_currentPosition++;
	entry = _undoStack[_currentPosition];
	diff = loadDiffFromFile(entry.getFileOffset(), entry.getDataSize());
	
	// 現在の画像データを読み取り
	currentData = new unsigned char[_imageWidth * _imageHeight * 4];
	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	glReadPixels(0, 0, _imageWidth, _imageHeight, GL_RGBA, GL_UNSIGNED_BYTE, currentData);
	
	// afterデータで復元（差分領域のみ）
	ImageDiff::decompressRLE(diff.getAfterData(), currentData, _imageWidth, _imageHeight,
						   diff.getMinX(), diff.getMinY(), diff.getMaxX(), diff.getMaxY());
	
	// 差分領域のみをテクスチャに更新
	glBindTexture(GL_TEXTURE_2D, texId);
	regionWidth = diff.getMaxX() - diff.getMinX() + 1;
	regionHeight = diff.getMaxY() - diff.getMinY() + 1;
	
	// 差分領域のデータを抽出
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
	
	std::cout << "Redo applied. Position: " << _currentPosition << std::endl;
}

void	UndoSystem::workerFunction()
{
	while (!_shouldStop)
	{
		std::unique_lock<std::mutex> lock(_queueMutex);
		_taskCondition.wait(lock, [this] { return !_taskQueue.empty() || _shouldStop; });
		
		if (_shouldStop)
			break;
		
		UndoTask task = _taskQueue.front();
		_taskQueue.pop();
		lock.unlock();
		
		processTask(task);
	}
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

ImageDiff	UndoSystem::loadDiffFromFile(long offset, size_t /*size*/)
{
	std::ifstream	file(UNDO_FILE_NAME, std::ios::binary);
	ImageDiff		diff;

	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open undo file for reading.");
	}

	file.seekg(offset);
	
	diff.deserialize(file);
	
	return (diff);
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