
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

    // there is na note there, so we should go there, and select
    seq->editor->selectAt(0, cvTest, shift);
    assertEQ(seq->context->cursorPitch(), cvTest);
    assertEQ(seq->context->cursorTime(), 0);
    assertEQ(seq->selection->size(), 1);
}

void testMidiEditorSelection()
{
    testSelectAt0(false);
    testSelectAt1(false);
}