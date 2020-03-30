
#include <assert.h>

#include "SqCommand.h"
#include "UndoRedoStack.h"

#ifndef __USE_VCV_UNDO
bool UndoRedoStack::canUndo() const
{
    return !undoList.empty() || !undo4List.empty();
}

bool UndoRedoStack::canRedo() const
{
    return !redoList.empty() || !redo4List.empty();
}

void UndoRedoStack::execute(MidiSequencerPtr seq, std::shared_ptr<SqCommand> cmd)
{
    cmd->execute(seq, nullptr);     // only used for unit tests, maybe we can get away with this
    undoList.push_front(cmd);
    redoList.clear();   
}

void UndoRedoStack::execute4(MidiSequencer4Ptr seq, std::shared_ptr<Sq4Command> cmd)
{
    cmd->execute(seq, nullptr);     // only used for unit tests, maybe we can get away with this
    undo4List.push_front(cmd);
    redo4List.clear();   
}

void UndoRedoStack::undo(MidiSequencerPtr seq)
{
    assert(canUndo());
    CommandPtr cmd = undoList.front();
    cmd->undo(seq, nullptr);
    undoList.pop_front();

    redoList.push_front(cmd);
}

void UndoRedoStack::undo4(MidiSequencer4Ptr seq)
{
    assert(canUndo());
    Command4Ptr cmd = undo4List.front();
    cmd->undo(seq, nullptr);
    undo4List.pop_front();

    redo4List.push_front(cmd);
}

void UndoRedoStack::redo(MidiSequencerPtr seq)
{
    assert(canRedo());
    CommandPtr cmd = redoList.front();
    cmd->execute(seq, nullptr);
    redoList.pop_front();

    undoList.push_front(cmd);
}

void UndoRedoStack::redo4(MidiSequencer4Ptr seq)
{
    assert(canRedo());
    Command4Ptr cmd = redo4List.front();
    cmd->execute(seq, nullptr);
    redo4List.pop_front();

    undo4List.push_front(cmd);
}

#endif