#pragma once

#include <memory>

class MidiTrack;
using MidiTrackPtr = std::shared_ptr<MidiTrack>;
class InteropClipboard
{
public:
    static void put(MidiTrackPtr);
    static MidiTrackPtr get();
};