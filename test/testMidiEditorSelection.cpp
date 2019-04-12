
#include "asserts.h"
#include "MidiSequencer.h"

// sequencer factory - helper function
static MidiSequencerPtr makeTest(bool empty = false)
{
    /**
     * makes a track of 8 1/4 notes, each of 1/8 note duration (50%).
     * pitch is ascending in semitones from 3:0 (c)
     */
    MidiSongPtr song = empty ?
        MidiSong::MidiSong::makeTest(MidiTrack::TestContent::empty, 0) :
        MidiSong::MidiSong::makeTest(MidiTrack::TestContent::eightQNotes, 0);
    MidiSequencerPtr sequencer = MidiSequencer::make(song);

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
   MidiSequencerPtr seq = makeTest();
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
    MidiSequencerPtr seq = makeTest();
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
    MidiSequencerPtr seq = makeTest();
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

static void testToggleSelection0()
{
    MidiSequencerPtr seq = makeTest();
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
    MidiSequencerPtr seq = makeTest();
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

    testToggleSelection0();
    testToggleSelection1();
}