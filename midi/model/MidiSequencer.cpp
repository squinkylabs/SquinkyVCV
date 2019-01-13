
#include "MidiSequencer.h"

int _mdb = 0;       // global instance counter

MidiSequencer::MidiSequencer(std::shared_ptr<MidiSong> sng) :
    selection( std::make_shared<MidiSelectionModel>()),
    editor( std::make_shared<MidiEditor>(sng, selection)),
    song(sng)
{
    ++_mdb;
}

MidiSequencer::~MidiSequencer()
{
    --_mdb;
}