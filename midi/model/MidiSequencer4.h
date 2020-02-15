#pragma once

#include <memory>

class MidiSong4;
using MidiSong4Ptr = std::shared_ptr<MidiSong4>;

/**
 * Similar to the MidiSequencer used in Seq++
 * basically a struct to hold all the data we need.
 */
class MidiSequencer4
{
public:
    std::shared_ptr<MidiSong4> song;
    static std::shared_ptr<MidiSequencer4> make(MidiSong4Ptr);
private:
    MidiSequencer4(MidiSong4Ptr);
    MidiSequencer4() = delete;
    MidiSequencer4(const MidiSequencer4&) = delete;
};

using MidiSequencer4Ptr = std::shared_ptr<MidiSequencer4>;