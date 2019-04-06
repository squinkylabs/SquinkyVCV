
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

void UndoRedoStack::execute(MidiSequencerPtr seq, std::shared_ptr<SqCommand> cmd)
{
    cmd->execute(seq);
    undoList.push_front(cmd);
    redoList.clear();   
}


void UndoRedoStack::undo(MidiSequencerPtr seq)
{
    assert(canUndo());
    CommandPtr cmd = undoList.front();
    cmd->undo(seq);
    undoList.pop_front();

    redoList.push_front(cmd);
}

void UndoRedoStack::redo(MidiSequencerPtr seq)
{
    assert(canRedo());
    CommandPtr cmd = redoList.front();
    cmd->execute(seq);
    redoList.pop_front();

    undoList.push_front(cmd);
}

#endif