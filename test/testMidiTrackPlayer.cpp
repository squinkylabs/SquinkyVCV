
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

static MidiSong4Ptr makeSong3(int trackNum)
{
    MidiSong4Ptr song = std::make_shared<MidiSong4>();
    MidiLocker lock(song->lock);
    MidiTrackPtr clip0 = MidiTrack::makeTest(MidiTrack::TestContent::oneQ1_75, song->lock);
    MidiTrackPtr clip1 = MidiTrack::makeTest(MidiTrack::TestContent::eightQNotesCMaj, song->lock);
    MidiTrackPtr clip2 = MidiTrack::makeTest(MidiTrack::TestContent::eightQNotesCMaj, song->lock);
    assertEQ(clip0->getLength(), 4.f);
    assertEQ(clip1->getLength(), 8.f);

    song->addTrack(trackNum, 0, clip0);
    song->addTrack(trackNum, 1, clip1);
    song->addTrack(trackNum, 2, clip2);
    return song;
}


/*
    durations = 1,2,2,2
*/
static MidiSong4Ptr makeSong4(int trackNum)
{
    MidiSong4Ptr song = std::make_shared<MidiSong4>();
    MidiLocker lock(song->lock);
    MidiTrackPtr clip0 = MidiTrack::makeTest(MidiTrack::TestContent::oneQ1_75, song->lock);
    MidiTrackPtr clip1 = MidiTrack::makeTest(MidiTrack::TestContent::eightQNotesCMaj, song->lock);
    MidiTrackPtr clip2 = MidiTrack::makeTest(MidiTrack::TestContent::eightQNotesCMaj, song->lock);
    MidiTrackPtr clip3 = MidiTrack::makeTest(MidiTrack::TestContent::eightQNotesCMaj, song->lock);
    assertEQ(clip0->getLength(), 4.f);
    assertEQ(clip1->getLength(), 8.f);

    song->addTrack(trackNum, 0, clip0);
    song->addTrack(trackNum, 1, clip1);
    song->addTrack(trackNum, 2, clip2);
    song->addTrack(trackNum, 3, clip3);
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
    printf("\n----- testLoop1\n");
    std::shared_ptr<IMidiPlayerHost4> host = std::make_shared<TestHost2>();
    MidiSong4Ptr song = makeSong(0);
    MidiTrackPlayer pl(host, 0, song);

    // Set up to loop the first section twice
    auto options0 = song->getOptions(0, 0);
    options0->repeatCount = 2;
    const float quantizationInterval = .01f;
    int x = pl.getCurrentRepetition();
    assertEQ(x, 0);                 // when stopped, always zero
    pl.reset(false);                     // for some reason we need to do this before we start
    pl.setRunningStatus(true);      // start it.

    /* I think the problem here is that we are in a strange state after setting play,
     * but before first clocks come through. We need to get this stuff into sync
     */

    x = pl.getCurrentRepetition();
    assertEQ(x, 1);                 //now we are playing first time
    
    bool played = pl.playOnce(4.1, quantizationInterval);     // play first rep + a bit

    assert(played);
    x = pl.getCurrentRepetition();
    assertEQ(x, 1);                                     // should still be playing first time
   
    played = pl.playOnce(4.1, quantizationInterval);    // play first rep + a bit
    assert(played);                                     // only one note in first loop (and one note off)
  
    played = pl.playOnce(4.1, quantizationInterval);    // play first rep + a bit
    assert(played);                                     // and the end event

    played = pl.playOnce(4.1, quantizationInterval);    // play first rep + a bit
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
    pl.reset(false);                     // for some reason we need to do this before we start
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
    Param param;
    pl.setPorts(&inputPort, &param);

    auto options0 = song->getOptions(0, 0);
    options0->repeatCount = 0;              // play forever
    const float quantizationInterval = .01f;
    pl.reset(false);                     // for some reason we need to do this before we start
    pl.setRunningStatus(true);      // start it.


    double endTime = 0;
    for (int iLoop = 0; iLoop < 10; ++iLoop) {
        endTime = 3.9 + iLoop * 4;
        play(pl, endTime, quantizationInterval);
        int expectedNotes = 1 + iLoop;
        assertEQ(2 * expectedNotes, host->gateChangeCount);
        // printf("end time was %.2f\n", endTime);
    }
    int x = host->gateChangeCount;
    inputPort.setVoltage(5, 0);     // send a pulse to channel 0
    pl.updateSampleCount(4);        // a few more process calls

    // above ends at 38.9
    // now should move to next
    auto getChangesBefore = host->gateChangeCount;
    double t = endTime + 4;          // one more bar

    play(pl, t, quantizationInterval);
    x = host->gateChangeCount;

    // we should have switched to the one with 8 notes in two bars.
    // but we only played the first bar
    assertEQ(x,  (getChangesBefore + 8));

}

static void testSwitchToNext2()
{
    std::shared_ptr<TestHost2> host = std::make_shared<TestHost2>();
    MidiSong4Ptr song = makeSong(0);
    MidiTrackPlayer pl(host, 0, song);

    Input inputPort;
    Param param;
    pl.setPorts(&inputPort, &param);

    {
        // set both section to play forever
        auto options0 = song->getOptions(0, 0);
        options0->repeatCount = 0;
        auto options1 = song->getOptions(0, 1);
        options1->repeatCount = 0;
    }
    const float quantizationInterval = .01f;
    pl.reset(false);                     // for some reason we need to do this before we start
    pl.setRunningStatus(true);      // start it.

    // play to middle of first bar
    play(pl, 2, quantizationInterval);
    int x = pl.getSection();
    assertEQ(x, 1);

    // cue up a switch to next section
    inputPort.setVoltage(5.f, 0);     // send a pulse to channel 0
    pl.updateSampleCount(4);        // a few more process calls
    inputPort.setVoltage(0.f, 0);
    pl.updateSampleCount(4);

    // play to start of next section
    play(pl, 4.1, quantizationInterval);
    x = pl.getSection();
    assertEQ(x, 2);

    // cue up a switch to next section. Since there is no next,
    // should wrap to first
    inputPort.setVoltage(5.f, 0);     // send a pulse to channel 0
    pl.updateSampleCount(4);        // a few more process calls
    inputPort.setVoltage(0.f, 0);
    pl.updateSampleCount(4);

    // play to start of next section
    play(pl, 4 + 8 + .1, quantizationInterval);
    x = pl.getSection();
    assertEQ(x, 1);
}

static void testSwitchToNextThenVamp()
{
    // make a song with three sections
    std::shared_ptr<TestHost2> host = std::make_shared<TestHost2>();
    MidiSong4Ptr song = makeSong3(0);
    MidiTrackPlayer pl(host, 0, song);

    Input inputPort;
    Param param;
    pl.setPorts(&inputPort, &param);

    {
        // set all section to play forever
        auto options0 = song->getOptions(0, 0);
        options0->repeatCount = 0;
        auto options1 = song->getOptions(0, 1);
        options1->repeatCount = 0;
        auto options2 = song->getOptions(0, 2);
        options2->repeatCount = 0;
    }
    const float quantizationInterval = .01f;
    pl.reset(false);                     // for some reason we need to do this before we start
    pl.setRunningStatus(true);      // start it.

    // play to middle of first bar
    play(pl, 2, quantizationInterval);
    int x = pl.getSection();
    assertEQ(x, 1);

    // cue up a switch to next section
    inputPort.setVoltage(5.f, 0);     // send a pulse to channel 0
    pl.updateSampleCount(4);        // a few more process calls
    inputPort.setVoltage(0.f, 0);
    pl.updateSampleCount(4);

    // play to start of next section
    play(pl, 4.1, quantizationInterval);
    x = pl.getSection();
    assertEQ(x, 2);


    // play to start of next section (should stick on 2)
    play(pl, 4 + 8 + .1, quantizationInterval);
    x = pl.getSection();
    assertEQ(x, 2);
#if 0
    // cue up a switch to prev section.
    // should wrap to second
    inputPort.setVoltage(5.f, 1);     // send a pulse to channel 0
    pl.updateSampleCount(4);        // a few more process calls
    inputPort.setVoltage(0.f, 1);
    pl.updateSampleCount(4);

      // play to start of next section
    play(pl, 4 + 4 + 8 + .1, quantizationInterval);
    x = pl.getSection();
    assertEQ(x, 3);
#endif
}

static void testSwitchToPrev()
{
    // make a song with three sections
    std::shared_ptr<TestHost2> host = std::make_shared<TestHost2>();
    MidiSong4Ptr song = makeSong3(0);
    MidiTrackPlayer pl(host, 0, song);

    Input inputPort;
    Param param;
    pl.setPorts(&inputPort, &param);

    {
        // set all section to play forever
        auto options0 = song->getOptions(0, 0);
        options0->repeatCount = 0;
        auto options1 = song->getOptions(0, 1);
        options1->repeatCount = 0;
        auto options2 = song->getOptions(0, 2);
        options2->repeatCount = 0;
    }
    const float quantizationInterval = .01f;
    pl.reset(false);                     // for some reason we need to do this before we start
    pl.setRunningStatus(true);      // start it.

    // play to middle of first bar
    play(pl, 2, quantizationInterval);
    int x = pl.getSection();
    assertEQ(x, 1);

    // cue up a switch to next section
    inputPort.setVoltage(5.f, 0);     // send a pulse to channel 0
    pl.updateSampleCount(4);        // a few more process calls
    inputPort.setVoltage(0.f, 0);
    pl.updateSampleCount(4);

    // play to start of next section
    play(pl, 4.1, quantizationInterval);
    x = pl.getSection();
    assertEQ(x, 2);


    // cue up a switch to prev section.
    // should go back to first
    inputPort.setVoltage(5.f, 1);     // send a pulse to channel 0
    pl.updateSampleCount(4);        // a few more process calls
    inputPort.setVoltage(0.f, 1);
    pl.updateSampleCount(4);

    // play to start of next section (back to 1)
    play(pl, 4 + 8 + .1, quantizationInterval);
    x = pl.getSection();
    assertEQ(x, 1);
#if 0
    // cue up a switch to prev section.
    // should wrap to second
    inputPort.setVoltage(5.f, 1);     // send a pulse to channel 0
    pl.updateSampleCount(4);        // a few more process calls
    inputPort.setVoltage(0.f, 1);
    pl.updateSampleCount(4);

      // play to start of next section
    play(pl, 4 + 4 + 8 + .1, quantizationInterval);
    x = pl.getSection();
    assertEQ(x, 3);
#endif
}


static void testRepetition()
{
    // make a song with three sections
    std::shared_ptr<TestHost2> host = std::make_shared<TestHost2>();
    MidiSong4Ptr song = makeSong(0);
    MidiTrackPlayer pl(host, 0, song);

    Input inputPort;
    Param param;
    pl.setPorts(&inputPort, &param);

    {
        auto options0 = song->getOptions(0, 0);
        options0->repeatCount = 1;
        auto options1 = song->getOptions(0, 1);
        options1->repeatCount = 4;
      
    }
    const float quantizationInterval = .01f;
    pl.reset(false);                     // for some reason we need to do this before we start
    pl.setRunningStatus(true);      // start it.

    // start first loop
    play(pl, .5, quantizationInterval);
    int x = pl.getCurrentRepetition();
    assertEQ(x, 1);

    // start of play to second
    play(pl, 8.5, quantizationInterval);
    x = pl.getCurrentRepetition();
    assertEQ(x, 1);

     // second rep of second
    play(pl, 8 + 8.5, quantizationInterval);
    x = pl.getCurrentRepetition();
    assertEQ(x, 2);

}

static void testRandomSwitch()
{
       // make a song with four sections
    std::shared_ptr<TestHost2> host = std::make_shared<TestHost2>();
    MidiSong4Ptr song = makeSong4(0);
    MidiTrackPlayer pl(host, 0, song);

    Input inputPort;
    Param param;
    pl.setPorts(&inputPort, &param);

    const float quantizationInterval = .01f;
    pl.setRunningStatus(true);      // start it.
    assertEQ(pl.getNextSectionRequest(), 0);

    pl.setNextSectionRequest(4);
    assertEQ(pl.getNextSectionRequest(), 4);

    pl.setNextSectionRequest(3);
    assertEQ(pl.getNextSectionRequest(), 3);

    // not running yet;
    assertEQ(pl.getSection(), 0);

    // now service outstanding request
    pl.playOnce(.1, quantizationInterval);
    assertEQ(pl.getNextSectionRequest(), 0);

    assertEQ(pl.getSection(), 3);

}

static void testMissingSection()
{
    // make a song with four sections
    std::shared_ptr<TestHost2> host = std::make_shared<TestHost2>();
    MidiSong4Ptr song = makeSong(0);
    MidiTrackPlayer pl(host, 0, song);

    Input inputPort;
    Param param;
    pl.setPorts(&inputPort, &param);

    const float quantizationInterval = .01f;
    pl.setNextSectionRequest(4);        // doesn't exist
    assertEQ(pl.getNextSectionRequest(), 1);        // so we wrap around to first

    song->addTrack(0, 0, nullptr);          // let's remove first clip

    pl.playOnce(.1, quantizationInterval); // play a bit to prime the pump
    assertEQ(pl.getSection(), 2);           // we should skip over the missing one.       

}

static void testHardReset()
{
    // we should set up, play a little, stop, reset, play again, find we are at start.
    // make a song with four sections
    std::shared_ptr<TestHost2> host = std::make_shared<TestHost2>();
    MidiSong4Ptr song = makeSong4(0);
    MidiTrackPlayer pl(host, 0, song);

    Input inputPort;
    Param param;
    pl.setPorts(&inputPort, &param);

    const float quantizationInterval = .01f;
    pl.setRunningStatus(true);      // start it.
  

    pl.setNextSectionRequest(4);
    assertEQ(pl.getNextSectionRequest(), 4);

    // now service outstanding request
    pl.playOnce(.1, quantizationInterval);
    assertEQ(pl.getNextSectionRequest(), 0);
    assertEQ(pl.getSection(), 4);

    // stop, and reset
    pl.setRunningStatus(false);
    pl.reset(true);         // set back to section 1

    // could assert the next is now 1??
    pl.playOnce(.2, quantizationInterval);
    assertEQ(pl.getSection(), 1);
}

// static void play(MidiTrackPlayer& pl, double time, float quantize)

static void testPlayThenReset()
{
    // make a song with four sections
    std::shared_ptr<TestHost2> host = std::make_shared<TestHost2>();
    MidiSong4Ptr song = makeSong4(0);
    MidiTrackPlayer pl(host, 0, song);

    Input inputPort;
    Param param;
    pl.setPorts(&inputPort, &param);

    const float quantizationInterval = .01f;
    pl.setRunningStatus(true);      // start it.

    // first and part of second
    play(pl, 3, quantizationInterval);
    assertEQ(pl.getSection(), 1);
    play(pl, 3.9, quantizationInterval);
    assertEQ(pl.getSection(), 1);
    play(pl, 4.1, quantizationInterval);
    assertEQ(pl.getSection(), 2);

    // reset
    pl.setRunningStatus(false);
    pl.reset(true);
    pl.setRunningStatus(true);

    // should first and part of second just like before
    play(pl, 3, quantizationInterval);
    assertEQ(pl.getSection(), 1);
    play(pl, 3.9, quantizationInterval);
    assertEQ(pl.getSection(), 1);
    play(pl, 4.1, quantizationInterval);
    assertEQ(pl.getSection(), 2);

}


static void testPlayThenResetSeek()
{
    // make a song with four sections
    std::shared_ptr<TestHost2> host = std::make_shared<TestHost2>();
    MidiSong4Ptr song = makeSong4(0);
    MidiTrackPlayer pl(host, 0, song);

    Input inputPort;
    Param param;
    pl.setPorts(&inputPort, &param);

    const float quantizationInterval = .01f;
    pl.setRunningStatus(true);      // start it.

    // first and part of second
    play(pl, 3, quantizationInterval);
    assertEQ(pl.getSection(), 1);
    play(pl, 3.9, quantizationInterval);
    assertEQ(pl.getSection(), 1);
    play(pl, 4.1, quantizationInterval);
    assertEQ(pl.getSection(), 2);

    // reset and request last section
    pl.setRunningStatus(false);
    pl.reset(true);
    pl.setNextSectionRequest(4);
    pl.setRunningStatus(true);                  // will put startup req in queue.

    // now will startup in section 4, which is 2 bars long
    play(pl, 3, quantizationInterval);        
    assertEQ(pl.getSection(), 4);
    play(pl, 3.9, quantizationInterval);
    assertEQ(pl.getSection(), 4);
    play(pl, 4.1, quantizationInterval);
    assertEQ(pl.getSection(), 4);
    play(pl, 7.9, quantizationInterval);
    assertEQ(pl.getSection(), 4);

    play(pl, 8.1, quantizationInterval);
    assertEQ(pl.getSection(), 1);
}

void testMidiTrackPlayer()
{
    testCanCall();
    testLoop1();
    testForever();
    testSwitchToNext();
    testSwitchToNext2();
    testSwitchToNextThenVamp();
    testSwitchToPrev();
    testRepetition();
    testRandomSwitch();
    testMissingSection();
    testHardReset();
    testPlayThenReset();
    testPlayThenResetSeek();
}