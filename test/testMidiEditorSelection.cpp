
#include "asserts.h"
#include "MidiLock.h"
#include "MidiSequencer.h"
#include "TestAuditionHost.h"
#include "TestSettings.h"

static MidiTrackPtr makeSpiralTrack(std::shared_ptr<MidiLock> lock)
{
    auto track = std::make_shared<MidiTrack>(lock);
 //   int semi = 0;
    MidiEvent::time_t time = 0;


    for (int i = 0; i < 8; ++i) {
        MidiNoteEventPtr ev = std::make_shared<MidiNoteEvent>();
        ev->startTime = time;
        
        ev->duration = .5;
        int semi = 0;
        switch (i) {
            case 0: 
                semi = 0;
                break;
            case 1:
                semi = 1;
                break;
            case 2:
                semi = -1;
                break;
            case 3:
                semi = 2;
                break;
            case 4:
                semi = -2;
                break;
            case 5:
                semi = 3;
                break;
            case 6:
                semi = -3;
                break;
            case 7:
                semi = 4;
                break;
            default:
                assert(false);
        }
        ev->setPitch(3, semi);

        track->insertEvent(ev);
        time += 1;
    }

    track->insertEnd(time);

    return track;
}

static MidiSongPtr makeSpiralSong()
{
    MidiSongPtr song = std::make_shared<MidiSong>();
    MidiLocker l(song->lock);
    auto track = makeSpiralTrack(song->lock);
    song->addTrack(0, track);
    song->assertValid();
    return song;
}

// sequencer factory - helper function
//  0 = eightNotes
//  1 = empty
// 2 = spiral
static MidiSequencerPtr makeTest2(int type)
{
    /**
     * makes a track of 8 1/4 notes, each of 1/8 note duration (50%).
     * pitch is ascending in semitones from 3:0 (c)
     */
    MidiSongPtr song;
    switch (type) {
        case 0:
            song = MidiSong::MidiSong::makeTest(MidiTrack::TestContent::eightQNotes, 0);
            break;
        case 1:
            song = MidiSong::MidiSong::makeTest(MidiTrack::TestContent::empty, 0);
            break;
        case 2:
            song = makeSpiralSong();
            break;
        default:
            assert(false);

    }
  
    MidiSequencerPtr sequencer = MidiSequencer::make(
        song, 
        std::make_shared<TestSettings>(),
        std::make_shared<TestAuditionHost>());

    sequencer->context->setTrackNumber(0);
    sequencer->context->setStartTime(0);
    sequencer->context->setEndTime(
        sequencer->context->startTime() + 8);
    sequencer->context->setPitchLow(PitchUtils::pitchToCV(3, 0));
    sequencer->context->setPitchHi(PitchUtils::pitchToCV(5, 0));

    sequencer->assertValid();
    return sequencer;
}

static void testSelectAt0(bool shift)
{
   MidiSequencerPtr seq = makeTest2(0);
   float cv0= PitchUtils::pitchToCV(3, 0);
   float cvTest = cv0 + 10 * PitchUtils::semitone;

   // there is no note there, so we should go there, but not select
   seq->editor->selectAt(0, cvTest, shift);
   assertEQ(seq->context->cursorPitch(), cvTest);
   assertEQ(seq->context->cursorTime(), 0);
   assert(seq->selection->empty());
}

static void testSelectAt1(bool shift)
{
    MidiSequencerPtr seq = makeTest2(0);
    float cv0 = PitchUtils::pitchToCV(3, 0);
    float cvTest = cv0;

    // there is a note there, so we should go there, and select
    seq->editor->selectAt(0, cvTest, shift);
    assertEQ(seq->context->cursorPitch(), cvTest);
    assertEQ(seq->context->cursorTime(), 0);
    assertEQ(seq->selection->size(), 1);
}

static void testSelectAt2(bool shift)
{
    MidiSequencerPtr seq = makeTest2(0);
    float cv0 = PitchUtils::pitchToCV(3, 0);
    float cvTest = cv0;

    // there is a note there, so we should go there, and select
    seq->editor->selectAt(0, cvTest, shift);
    assertEQ(seq->context->cursorPitch(), cvTest);
    assertEQ(seq->context->cursorTime(), 0);
    assertEQ(seq->selection->size(), 1);

    cvTest += PitchUtils::semitone;
    seq->editor->selectAt(1, cvTest, shift);
    int expectSelect = (shift) ? 2 : 1;
    assertEQ(seq->context->cursorPitch(), cvTest);
    assertEQ(seq->context->cursorTime(), 1);
    assertEQ(seq->selection->size(), expectSelect);

    cvTest += PitchUtils::semitone;
    seq->editor->selectAt(2, cvTest, shift);
    expectSelect = (shift) ? 3 : 1;
    assertEQ(seq->context->cursorPitch(), cvTest);
    assertEQ(seq->context->cursorTime(), 2);
    assertEQ(seq->selection->size(), expectSelect);
}


static void testExtend1()
{
    MidiSequencerPtr seq = makeTest2(2);

    seq->editor->selectAt(0, PitchUtils::pitchToCV(3, 0), true);
    assertEQ(seq->selection->size(), 1);

    seq->editor->selectAt(1, PitchUtils::pitchToCV(3, 1), true);
    assertEQ(seq->selection->size(), 2);

    seq->editor->selectAt(2, PitchUtils::pitchToCV(3, -1), true);
    assertEQ(seq->selection->size(), 3);
}


static void testExtend2()
{
    MidiSequencerPtr seq = makeTest2(2);

    seq->editor->selectAt(0, PitchUtils::pitchToCV(3, 0), true);
    assertEQ(seq->selection->size(), 1);

    seq->editor->selectAt(2, PitchUtils::pitchToCV(3, -1), true);
    assertEQ(seq->selection->size(), 2);
}


static void testExtend3()
{
    MidiSequencerPtr seq = makeTest2(2);

    seq->editor->selectAt(0, PitchUtils::pitchToCV(3, 0), true);
    assertEQ(seq->selection->size(), 1);

    seq->editor->selectAt(3, PitchUtils::pitchToCV(3, 2), true);
    assertEQ(seq->selection->size(), 3);
}

static void testExtend4()
{
    MidiSequencerPtr seq = makeTest2(2);

    // select first note
    seq->editor->selectAt(0, PitchUtils::pitchToCV(3, 0), true);
    assertEQ(seq->selection->size(), 1);

    // then in outer space
    seq->editor->selectAt(50, PitchUtils::pitchToCV(6,0), true);
    assertEQ(seq->selection->size(), 5);
}

static void testToggleSelection0()
{
    MidiSequencerPtr seq = makeTest2(0);
    float cv0 = PitchUtils::pitchToCV(3, 0);
    float cvTest = cv0;

    // there is a note at t=0, , toggle it on
    seq->editor->toggleSelectionAt(0, cvTest);
    assertEQ(seq->context->cursorPitch(), cvTest);
    assertEQ(seq->context->cursorTime(), 0);
    assertEQ(seq->selection->size(), 1);

     // now toggle it off
    seq->editor->toggleSelectionAt(0, cvTest);
    assertEQ(seq->context->cursorPitch(), cvTest);
    assertEQ(seq->context->cursorTime(), 0);
    assertEQ(seq->selection->size(), 0);
}


static void testToggleSelection1()
{
    MidiSequencerPtr seq = makeTest2(0);
    float cv0 = PitchUtils::pitchToCV(3, 0);
    float cvTest = cv0;

    // there is a note at t=0, , toggle it on
    seq->editor->toggleSelectionAt(0, cvTest);
    assertEQ(seq->context->cursorPitch(), cvTest);
    assertEQ(seq->context->cursorTime(), 0);
    assertEQ(seq->selection->size(), 1);

    // now at t1
    cvTest += PitchUtils::semitone;
    seq->editor->toggleSelectionAt(1, cvTest);
    assertEQ(seq->context->cursorPitch(), cvTest);
    assertEQ(seq->context->cursorTime(), 1);
    assertEQ(seq->selection->size(), 2);

     // now at t3
    cvTest += PitchUtils::semitone;
    seq->editor->toggleSelectionAt(2, cvTest);
    assertEQ(seq->context->cursorPitch(), cvTest);
    assertEQ(seq->context->cursorTime(), 2);
    assertEQ(seq->selection->size(), 3);

    // now toggle middle off
    cvTest -= PitchUtils::semitone;
    seq->editor->toggleSelectionAt(1, cvTest);
    assertEQ(seq->context->cursorPitch(), cvTest);
    assertEQ(seq->context->cursorTime(), 1);
    assertEQ(seq->selection->size(), 2);
}

void testMidiEditorSelection()
{
    testSelectAt0(false);
    testSelectAt1(false);
    testSelectAt2(false);

    testSelectAt0(true);
    testSelectAt1(true);
    testSelectAt2(true);

    testExtend1();
    testExtend2();
    testExtend3();
    testExtend4();

    testToggleSelection0();
    testToggleSelection1();
}