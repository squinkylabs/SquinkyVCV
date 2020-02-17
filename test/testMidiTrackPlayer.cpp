
#include "MidiSong4.h"
#include "MidiTrack4Options.h"
#include "MidiTrackPlayer.h"
#include "TestHost2.h"
#include "asserts.h"

/**
 * Makes a multi section test song.
 * First section is one bar long, and has one quarter note in it.
 * Second section is two bars and has 8 1/8 notes in it (c...c)
 */
static MidiSong4Ptr makeSong(int trackNum)
{
    MidiSong4Ptr song = std::make_shared<MidiSong4>();
    MidiLocker lock(song->lock);
    MidiTrackPtr clip0 = MidiTrack::makeTest(MidiTrack::TestContent::oneQ1_75, song->lock);
    MidiTrackPtr clip1 = MidiTrack::makeTest(MidiTrack::TestContent::eightQNotesCMaj, song->lock);
    assertEQ(clip0->getLength(), 4.f);
    assertEQ(clip1->getLength(), 8.f);

    song->addTrack(trackNum, 0, clip0);
    song->addTrack(trackNum, 1, clip1);
    return song;
}

static void play(MidiTrackPlayer& pl, double time, float quantize)
{
    while (pl.playOnce(time, quantize)) {

    }
}

static void testCanCall()
{
    std::shared_ptr<IMidiPlayerHost4> host = std::make_shared<TestHost2>();
    std::shared_ptr<MidiSong4> song = std::make_shared<MidiSong4>();
    MidiTrackPlayer pl(host, 0, song);

    const float quantizationInterval = .01f;
    bool b = pl.playOnce(1, quantizationInterval);
    assert(!b);
}

static void testLoop1()
{
    std::shared_ptr<IMidiPlayerHost4> host = std::make_shared<TestHost2>();
    MidiSong4Ptr song = makeSong(0);
    MidiTrackPlayer pl(host, 0, song);

    auto options0 = song->getOptions(0, 0);
    options0->repeatCount = 2;
    const float quantizationInterval = .01f;
    int x = pl.getCurrentRepetition();
    assertEQ(x, 0);                 // when stopped, always zero
    pl.reset();                     // for some reason we need to do this before we start
    pl.setRunningStatus(true);      // start it.
    x = pl.getCurrentRepetition();
    assertEQ(x, 1);                 //now we are playing first time
    
    bool played = pl.playOnce(4.1, quantizationInterval);     // play first rep + a bit
    assert(played);
    played = pl.playOnce(4.1, quantizationInterval);     // play first rep + a bit
    assert(played);                        // only one note in first loop (and one note off)
  
    played = pl.playOnce(4.1, quantizationInterval);     // play first rep + a bit
    assert(played);                        // and the end event

    played = pl.playOnce(4.1, quantizationInterval);     // play first rep + a bit
    assert(!played);

    x = pl.getCurrentRepetition();
    assertEQ(x, 2);                 //now we are playing second time
}

static void testForever()
{
    std::shared_ptr<TestHost2> host = std::make_shared<TestHost2>();
    MidiSong4Ptr song = makeSong(0);
    MidiTrackPlayer pl(host, 0, song);

    auto options0 = song->getOptions(0, 0);
    options0->repeatCount = 0;              // play forever
    const float quantizationInterval = .01f;
    pl.reset();                     // for some reason we need to do this before we start
    pl.setRunningStatus(true);      // start it.

    // we set it to "forever", so let's see if it can play 100 times.
    for (int iLoop = 0; iLoop < 100; ++iLoop) {
        double endTime = 3.9 + iLoop * 4;
        play(pl, endTime, quantizationInterval);
        int expectedNotes = 1 + iLoop;
        assertEQ(2 * expectedNotes, host->gateChangeCount);
    }
}

static void testSwitchToNext()
{
    std::shared_ptr<TestHost2> host = std::make_shared<TestHost2>();
    MidiSong4Ptr song = makeSong(0);
    MidiTrackPlayer pl(host, 0, song);

    Input inputPort;
    pl.setInputPort(&inputPort);

    auto options0 = song->getOptions(0, 0);
    options0->repeatCount = 0;              // play forever
    const float quantizationInterval = .01f;
    pl.reset();                     // for some reason we need to do this before we start
    pl.setRunningStatus(true);      // start it.


    double endTime = 0;
    for (int iLoop = 0; iLoop < 10; ++iLoop) {
        endTime = 3.9 + iLoop * 4;
        play(pl, endTime, quantizationInterval);
        int expectedNotes = 1 + iLoop;
        assertEQ(2 * expectedNotes, host->gateChangeCount);
        printf("end time was %.2f\n", endTime);
    }
    int x = host->gateChangeCount;
    inputPort.setVoltage(5, 0);     // send a pulse to channel 0
    pl.updateSampleCount(4);        // a few more process calls

    // above ends at 38.9
    // now should move to next
    auto getChangesBefore = host->gateChangeCount;
    double t = endTime + 4;          // one more bar
  //  assert(false);
    play(pl, t, quantizationInterval);
    x = host->gateChangeCount;

    // we shoud have switched to the one with 8 notes in the bar.
    assertEQ(x,  (getChangesBefore + 16));

}

void testMidiTrackPlayer()
{
    testCanCall();
    testLoop1();
    testForever();
    testSwitchToNext();
}