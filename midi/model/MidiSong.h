#pragma once

#include "MidiTrack.h"
#include <vector>
#include <memory>

class MidiSong;
class MidiLock;

using MidiSongPtr = std::shared_ptr<MidiSong>;

class SubrangeLoop
{
public:
    SubrangeLoop(bool b, float s, float e) : enabled(b), startTime(s), endTime(e)
    {
    }
    SubrangeLoop() = default;
    bool operator != (const SubrangeLoop& other) const
    {
        return enabled != other.enabled ||
            startTime != other.startTime ||
            endTime != other.endTime;
    }

    bool enabled = false;
    float startTime = 0;
    float endTime = 0;
};

class MidiSong
{
public:
    MidiSong();
    ~MidiSong();

    std::shared_ptr<MidiTrack> getTrack(int index);
    std::shared_ptr<const MidiTrack> getTrackConst(int index) const;
    void createTrack(int index);
    /** like create track, but passes in the track
     */
    void addTrack(int index, std::shared_ptr<MidiTrack>);
   
    void assertValid() const;

    /**
     * returns -1 if no tracks exist
     */
    int getHighestTrackNumber() const;

    bool trackExists(int tkNum) const;

    /**
     * factory method to generate test content
     */
    static MidiSongPtr makeTest(MidiTrack::TestContent, int trackNumber);

    std::shared_ptr<MidiLock> lock;

    const SubrangeLoop& getLoop();
    void setLoop(const SubrangeLoop&);
private:
    std::vector<std::shared_ptr<MidiTrack>> tracks;
    SubrangeLoop loop;
};

