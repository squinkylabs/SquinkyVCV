
#include "MidiSequencer.h"

int _mdb = 0;       // global instance counter

MidiSequencer::MidiSequencer(std::shared_ptr<MidiSong>) :
    selection( std::make_shared<MidiSelectionModel>()),
    editor( std::make_shared<MidiEditor>())
{
    ++_mdb;
}

MidiSequencer::~MidiSequencer()
{
    --_mdb;
}