#pragma once

class MidiSong4;

/**
 * Similar to the MidiSeqeuencer used in Seq++
 * basically a struct to hold all the data we need.
 */
class MidiSequencer4
{
public:
    std::shared_ptr<MidiSong4> song;

};

using MidiSequencer4Ptr = std::shared_ptr<MidiSequencer4>;