
#include "MidiSequencer4.h"
#include "UndoRedoStack.h"

MidiSequencer4::MidiSequencer4(MidiSong4Ptr s) : song(s), undo(std::make_shared<UndoRedoStack>())
{

}

MidiSequencer4Ptr MidiSequencer4::make(MidiSong4Ptr s)
{
    MidiSequencer4Ptr seq(new MidiSequencer4(s));
    return seq;
}