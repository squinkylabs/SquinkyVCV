
#include <assert.h>

#include "SqCommand.h"
#include "UndoRedoStack.h"

#ifndef __USE_VCV_UNDO
bool UndoRedoStack::canUndo() const
{
    return !undoList.empty();
}

bool UndoRedoStack::canRedo() const
{
    return !redoList.empty();
}

void UndoRedoStack::execute(std::shared_ptr<SqCommand> cmd)
{
    cmd->execute();
    undoList.push_front(cmd);
    redoList.clear();   
}

void UndoRedoStack::undo()
{
    assert(canUndo());
    CommandPtr cmd = undoList.front();
    cmd->undo();
    undoList.pop_front();

    redoList.push_front(cmd);
}

void UndoRedoStack::redo()
{
    assert(canRedo());
    CommandPtr cmd = redoList.front();
    cmd->execute();
    redoList.pop_front();

    undoList.push_front(cmd);
}
#endif