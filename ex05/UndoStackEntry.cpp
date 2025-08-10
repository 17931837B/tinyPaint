#include "UndoStackEntry.hpp"

UndoStackEntry::UndoStackEntry(): _fileOffset(0), _dataSize(0), _isValid(false)
{

}

UndoStackEntry::UndoStackEntry(long offset, size_t size): _fileOffset(offset), _dataSize(size), _isValid(true)
{

}

long	UndoStackEntry::getFileOffset() const
{
	return (_fileOffset);
}

size_t	UndoStackEntry::getDataSize() const
{
	return (_dataSize);
}

bool	UndoStackEntry::getIsValid() const
{
	return (_isValid);
}

void	UndoStackEntry::setIsValid(bool valid)
{
	_isValid = valid;
}

void	UndoStackEntry::setFileOffset(long offset)
{
	_fileOffset = offset;
}

void	UndoStackEntry::setDataSize(size_t size)
{
	_dataSize = size;
}