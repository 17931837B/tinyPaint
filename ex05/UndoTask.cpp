#include "UndoTask.hpp"

UndoTask::UndoTask(Type t) : _type(t), _stackIndex(-1)
{

}

UndoTask::UndoTask(Type t, const ImageDiff& d, int idx) : _type(t), _diff(d), _stackIndex(idx)
{

}

UndoTask::Type	UndoTask::getType() const
{
	return (_type);
}

const ImageDiff&	UndoTask::getDiff() const
{
	return (_diff);
}

int	UndoTask::getStackIndex() const
{
	return (_stackIndex);
}

void	UndoTask::setType(Type t)
{
	_type = t;
}

void	UndoTask::setDiff(const ImageDiff& d)
{
	_diff = d;
}

void	UndoTask::setStackIndex(int idx)
{
	_stackIndex = idx;
}