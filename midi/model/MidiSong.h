#pragma once

#include <vector>
#include <memory>

class MidiTrack;
class MidiSong
{
public:
    std::shared_ptr<MidiTrack> getTrack(int index);
    std::shared_ptr<const MidiTrack> getTrack(int index) const;
    void createTrack(int index);
private:
    std::vector<std::shared_ptr<MidiTrack>> tracks;
};

using MidiSongPtr = std::shared_ptr<MidiSong>;