#pragma once

#include "MidiSelectionModel.h"
#include "MidiSong.h"
#include <memory>

class MidiSong;

class MidiSequencer
{
public:
    /** constructor takes a song to edit
    */
    MidiSequencer(std::shared_ptr<MidiSong>);

    MidiSelectionModelPtr const selection;
    MidiSongPtr const song;
};

using MidiSequencerPtr = std::shared_ptr<MidiSequencer>;