#ifndef UNDOSTACKENTRY_HPP
#define UNDOSTACKENTRY_HPP

#include <cstddef>

class	UndoStackEntry
{
	private:
		long	_fileOffset;
		size_t	_dataSize;
		bool	_isValid;
	public:
		UndoStackEntry();
		UndoStackEntry(long offset, size_t	size);
		long	getFileOffset() const;
		size_t	getDataSize() const;
		bool	getIsValid() const;
		void	setIsValid(bool valid);
		void	setFileOffset(long offset);
		void	setDataSize(size_t size);
};

#endif