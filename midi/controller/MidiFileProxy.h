#pragma once

#include <memory>
#include <string>

namespace smf {
    class MidiFile;
};

class MidiSong;
class MidiTrack;

using MidiSongPtr = std::shared_ptr<MidiSong>;
using MidiTrackPtr = std::shared_ptr<MidiTrack>;
class MidiFileProxy
{
public:
    MidiFileProxy() = delete;
    static MidiSongPtr load(const std::string& filename);
    static MidiTrackPtr getFirst(MidiSongPtr song, smf::MidiFile&);
    static bool save(MidiSongPtr song, char* filePath);
};