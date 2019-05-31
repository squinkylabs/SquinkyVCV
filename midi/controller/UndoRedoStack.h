#pragma once

#include <memory>

class SqCommand;
class MidiSequencer;
using MidiSequencerPtr = std::shared_ptr<MidiSequencer>;

#if defined(__PLUGIN) && defined(__V1x)
#define __USE_VCV_UNDO
#endif

#ifdef __USE_VCV_UNDO

class UndoRedoStack
{
public:
    // execute the command, make undo record
    void execute(MidiSequencerPtr, std::shared_ptr<SqCommand>);
    void setModuleId(int);
private:
    int moduleId=-1;
};

using UndoRedoStackPtr = std::shared_ptr<UndoRedoStack>;

#else

#include <memory>
#include <list>

class UndoRedoStack
{
public:
    bool canUndo() const;
    bool canRedo() const;

    // execute the command, make undo record
    void execute(MidiSequencerPtr, std::shared_ptr<SqCommand>);
    void undo(MidiSequencerPtr);
    void redo(MidiSequencerPtr);

private:

    std::list<std::shared_ptr<SqCommand>> undoList;
    std::list<std::shared_ptr<SqCommand>> redoList;

};

using UndoRedoStackPtr = std::shared_ptr<UndoRedoStack>;
#endif
