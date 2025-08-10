#ifndef UNDOTASK_HPP
#define UNDOTASK_HPP

#include "ImageDiff.hpp"

class UndoTask
{
	public:
		enum Type { SAVE_DIFF, APPLY_UNDO, APPLY_REDO };
	private:
		Type		_type;
		ImageDiff	_diff;
		int			_stackIndex;
	public:
		UndoTask(Type t);
		UndoTask(Type t, const ImageDiff& d, int idx = -1);
		Type				getType() const;
		const ImageDiff&	getDiff() const;
		int					getStackIndex() const;
		void				setType(Type t);
		void				setDiff(const ImageDiff& d);
		void				setStackIndex(int idx);
};

#endif