#include "MidiEvent.h"
#include "MidiPlayer.h"
#include "MidiSong.h"
#include "MidiTrack.h"

#include "asserts.h"


class TestHost : public MidiPlayer::IPlayerHost
{
public:
    void setGate(bool g) override
    {
        ++gateCount;
        gateState = g;
    }
    void setCV(float cv) override
    {
        ++cvCount;
        cvState = cv;
    }

    int gateCount = 0;
    bool gateState = false;
    int cvCount = 0;
    float cvState = 0;
};


static void test0()
{
    MidiSongPtr song = MidiSong::makeTest1();
    std::shared_ptr<TestHost> host = std::make_shared<TestHost>();
    MidiPlayer pl(host, song);
    pl.timeElapsed(.01f);
}

static MidiSongPtr makeSongOneQ()
{
    MidiSongPtr song = std::make_shared<MidiSong>();
    song->createTrack(0);
    MidiTrackPtr track = song->getTrack(0);

    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    note->startTime = 0;
    note->duration = .5;
    note->pitch = 2.f;
    track->insertEvent(note);
    track->insertEnd(1);

    return song;
}

static void test1()
{
    MidiSongPtr song = makeSongOneQ();
    std::shared_ptr<TestHost> host = std::make_shared<TestHost>();
    MidiPlayer pl(host, song);
    pl.timeElapsed(.1f);
    assertEQ(host->gateCount, 1);
    assertEQ(host->gateState, true);
    assertEQ(host->cvCount, 1);
    assertEQ(host->cvState, 2);

}


void testMidiPlayer()
{
    test0();
    test1();
}