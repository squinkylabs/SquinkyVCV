
#include "MidiPlayer4.h"
#include "MidiSong4.h"
#include "MidiTrack4Options.h"
#include "TestHost4.h"

#include "asserts.h"

/**
 * Makes a multi section test song.
 * First section is one bar long, and has one quarter note in it.
 * Second section is two vars and has 8 1/8 notes in it (c...c)
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

static void testTwoSections(int trackNum)
{
    MidiSong4Ptr song = makeSong(trackNum);
    std::shared_ptr<TestHost4> host = std::make_shared<TestHost4>();
    MidiPlayer4 pl(host, song);

    // before note, nothing
    const float quantizationInterval = .01f;       
    pl.updateToMetricTime(.8, quantizationInterval, true);
    assertEQ(host->gateChangeCount, 0);
    assertEQ(host->gateState[0], false);
  
    // after note, 1
    pl.updateToMetricTime(1.1, quantizationInterval, true);
    assertEQ(host->gateChangeCount, 1);
    host->assertOneActiveTrack(trackNum);
    assertEQ(host->cvValue[0], 7.5f);
    assertEQ(host->gateState[0], true);

    // after end of note
    pl.updateToMetricTime(3.8, quantizationInterval, true);
    assertEQ(host->gateChangeCount, 2);
    host->assertOneActiveTrack(trackNum);
    assertEQ(host->cvValue[0], 7.5f);
    assertEQ(host->gateState[0], false);

    // second section, first note (c at 0)
    pl.updateToMetricTime(4.1, quantizationInterval, true);
    host->assertOneActiveTrack(trackNum);
    assertEQ(host->gateChangeCount, 3);
    assertEQ(host->gateState[0], true);
    assertEQ(host->cvValue[0], PitchUtils::pitchToCV(3, PitchUtils::c));

    // second section, after first note (c at 0)
    pl.updateToMetricTime(4.6, quantizationInterval, true);
    host->assertOneActiveTrack(trackNum);
    assertEQ(host->gateChangeCount, 4);
    assertEQ(host->gateState[0], false);
    assertEQ(host->cvValue[0], PitchUtils::pitchToCV(3, PitchUtils::c));

    // second section, second note (d at 1)
    pl.updateToMetricTime(5, quantizationInterval, true);
    host->assertOneActiveTrack(trackNum);
    assertEQ(host->gateChangeCount, 5);
    assertEQ(host->gateState[0], true);
    assertEQ(host->cvValue[0], PitchUtils::pitchToCV(3, PitchUtils::d));

    // second section, after second note (d at 1)
    pl.updateToMetricTime(5.6, quantizationInterval, true);
    host->assertOneActiveTrack(trackNum);
    assertEQ(host->gateChangeCount, 6);
    assertEQ(host->gateState[0], false);
    assertEQ(host->cvValue[0], PitchUtils::pitchToCV(3, PitchUtils::d));

    // now all the way to the last note in the last section
    pl.updateToMetricTime(4 + 7, quantizationInterval, true);
    host->assertOneActiveTrack(trackNum);
    assertEQ(host->gateState[0], true);
    assertEQ(host->cvValue[0], PitchUtils::pitchToCV(4, PitchUtils::c));
}

static void testTwoSectionsLoop()
{
    const int trackNum = 0;
    MidiSong4Ptr song = makeSong(trackNum);
    std::shared_ptr<TestHost4> host = std::make_shared<TestHost4>();
    MidiPlayer4 pl(host, song);

    // go around loop, and back before first note
    const float loopTime = 4 + 8;
    const float quantizationInterval = .01f;
    pl.updateToMetricTime(loopTime + .8, quantizationInterval, true);
    host->assertOneActiveTrack(0);
    assertEQ(host->gateState[0], false);
    const int gate0 = host->gateChangeCount;

     // after note, 1
    pl.updateToMetricTime(loopTime + 1.1, quantizationInterval, true);
    assertEQ(host->gateChangeCount, gate0 + 1);
    host->assertOneActiveTrack(trackNum);
    assertEQ(host->cvValue[0], 7.5f);
    assertEQ(host->gateState[0], true);


    // after end of note
    pl.updateToMetricTime(loopTime + 3.8, quantizationInterval, true);
    assertEQ(host->gateChangeCount, gate0+2);
    host->assertOneActiveTrack(trackNum);
    assertEQ(host->cvValue[0], 7.5f);
    assertEQ(host->gateState[0], false);

}

static void testTwoSectionsRepeat1()
{
    const int trackNum = 0;
    MidiSong4Ptr song = makeSong(trackNum);

    auto options = song->getOptions(0, 0);
    options->repeatCount = 2;

    std::shared_ptr<TestHost4> host = std::make_shared<TestHost4>();
    MidiPlayer4 pl(host, song);

    // First section, first repeat
    // before note, nothing
    const float quantizationInterval = .01f;
    pl.updateToMetricTime(.8, quantizationInterval, true);
    assertEQ(host->gateChangeCount, 0);
    assertEQ(host->gateState[0], false);

    // after note, 1
    pl.updateToMetricTime(1.1, quantizationInterval, true);
    assertEQ(host->gateChangeCount, 1);
    host->assertOneActiveTrack(trackNum);
    assertEQ(host->cvValue[0], 7.5f);
    assertEQ(host->gateState[0], true);

    // after end of note
    pl.updateToMetricTime(3.8, quantizationInterval, true);
    assertEQ(host->gateChangeCount, 2);
    host->assertOneActiveTrack(trackNum);
    assertEQ(host->cvValue[0], 7.5f);
    assertEQ(host->gateState[0], false);


    // First section, second repeat
    // before note, nothing

    pl.updateToMetricTime(4.8, quantizationInterval, true);
    assertEQ(host->gateChangeCount, 2);
    assertEQ(host->gateState[0], false);

    // after note, 1
    pl.updateToMetricTime(4 +1.1, quantizationInterval, true);
    assertEQ(host->gateChangeCount, 3);
    host->assertOneActiveTrack(trackNum);
    assertEQ(host->cvValue[0], 7.5f);
    assertEQ(host->gateState[0], true);

    // after end of note
    pl.updateToMetricTime(4 + 3.8, quantizationInterval, true);
    assertEQ(host->gateChangeCount, 4);
    host->assertOneActiveTrack(trackNum);
    assertEQ(host->cvValue[0], 7.5f);
    assertEQ(host->gateState[0], false);

    // second section, first note (c at 0)
    pl.updateToMetricTime(4+ 4.1, quantizationInterval, true);
    host->assertOneActiveTrack(trackNum);
    assertEQ(host->gateChangeCount, 5);
    assertEQ(host->gateState[0], true);
    assertEQ(host->cvValue[0], PitchUtils::pitchToCV(3, PitchUtils::c));

    // second section, after first note (c at 0)
    pl.updateToMetricTime(4 + 4.6, quantizationInterval, true);
    host->assertOneActiveTrack(trackNum);
    assertEQ(host->gateChangeCount, 6);
    assertEQ(host->gateState[0], false);
    assertEQ(host->cvValue[0], PitchUtils::pitchToCV(3, PitchUtils::c));

    // second section, second note (d at 1)
    pl.updateToMetricTime(4 + 5, quantizationInterval, true);
    host->assertOneActiveTrack(trackNum);
    assertEQ(host->gateChangeCount, 7);
    assertEQ(host->gateState[0], true);
    assertEQ(host->cvValue[0], PitchUtils::pitchToCV(3, PitchUtils::d));

    // second section, after second note (d at 1)
    pl.updateToMetricTime(4 + 5.6, quantizationInterval, true);
    host->assertOneActiveTrack(trackNum);
    assertEQ(host->gateChangeCount, 8);
    assertEQ(host->gateState[0], false);
    assertEQ(host->cvValue[0], PitchUtils::pitchToCV(3, PitchUtils::d));

    // now all the way to the last note in the last section
    pl.updateToMetricTime(4 + 4 + 7, quantizationInterval, true);
    host->assertOneActiveTrack(trackNum);
    assertEQ(host->gateState[0], true);
    assertEQ(host->cvValue[0], PitchUtils::pitchToCV(4, PitchUtils::c));
}

static void testRepeatReset()
{
    const int trackNum = 0;
    MidiSong4Ptr song = makeSong(trackNum);

    // repeat second section twice.
    auto options = song->getOptions(0, 1);
    options->repeatCount = 2;
    const float quantizationInterval = .01f;

    std::shared_ptr<TestHost4> host = std::make_shared<TestHost4>();
    MidiPlayer4 pl(host, song);


    // Play the first section, verify it played one note
    pl.updateToMetricTime(3.8, quantizationInterval, true);
    assertEQ(host->gateChangeCount, 2);

    // Play a bit of second section, verify there are "lots of" notes.
    pl.updateToMetricTime(4 + 1.48f, quantizationInterval, true);
    assertGT(host->gateChangeCount, 4);

    // now reset the player and repeat
    pl.reset(true);
    pl.updateToMetricTime(.2, quantizationInterval, true);
    const int ct0 = host->gateChangeCount;

    // Play the first section, verify it played one note
    pl.updateToMetricTime(3.8, quantizationInterval, true);
    assertEQ(host->gateChangeCount, ct0 + 2);

    // Play a bit of second section, verify there are "lots of" notes.
    // (due to bug, this will fail, as there is still a stale repeat
    // count from section 2 before)
    pl.updateToMetricTime(4 + 1.48f, quantizationInterval, true);
    assertGT(host->gateChangeCount, ct0 + 4);
  
}

void testMidiPlayer4()
{
    testTwoSections(0);
    testTwoSections(3);
    testTwoSectionsLoop();
    testTwoSectionsRepeat1();
    testRepeatReset();
}