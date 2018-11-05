#pragma once

#include <vector>
#include <memory>

class MidiTrack;
class MidiSong;

using MidiSongPtr = std::shared_ptr<MidiSong>;

class MidiSong
{
public:
    std::shared_ptr<MidiTrack> getTrack(int index);
    std::shared_ptr<const MidiTrack> getTrack(int index) const;
    void createTrack(int index);

    //bool doesTrackExist(int index) const;

    /**
     * returns -1 if no tracks exist
     */
    int getHighestTrackNumber() const;

    /**
     * factory method to generate test content
     */
    static MidiSongPtr makeTest1();
private:
    std::vector<std::shared_ptr<MidiTrack>> tracks;

    /** like create track, but passes in the track
     */
    void addTrack(int index, std::shared_ptr<MidiTrack>);
};

