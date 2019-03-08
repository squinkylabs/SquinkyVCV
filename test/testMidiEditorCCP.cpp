
#include "MidiLock.h"
#include "MidiSequencer.h"
#include "SqClipboard.h"

#include <assert.h>

static int _trackNumber = 0;

// sequencer factory - helper function
static MidiSequencerPtr makeTest(bool empty = false)
{
    MidiSongPtr song = empty ?
        MidiSong::MidiSong::makeTest(MidiTrack::TestContent::empty, _trackNumber) :
        MidiSong::MidiSong::makeTest(MidiTrack::TestContent::eightQNotes, _trackNumber);
    MidiSequencerPtr sequencer = MidiSequencer::make(song);

    sequencer->context->setTrackNumber(_trackNumber);
    sequencer->context->setStartTime(0);
    sequencer->context->setEndTime(
        sequencer->context->startTime() + 8);
    sequencer->context->setPitchLow(PitchUtils::pitchToCV(3, 0));
    sequencer->context->setPitchHi(PitchUtils::pitchToCV(5, 0));

    sequencer->assertValid();
    return sequencer;
}


static void testCopy1()
{
    SqClipboard::clear();
    MidiSequencerPtr seq = makeTest(false);
    assertEQ(seq->context->getTrack()->size(), 9);
    seq->editor->selectNextNote();          // now first is selected
  
    assert(!SqClipboard::getTrackData());
    seq->editor->copy();

    assertEQ(seq->context->getTrack()->size(), 9);
    auto clip = SqClipboard::getTrackData();
    assert(clip);
    clip->track->assertValid();
    assertEQ(clip->track->size(), 2);       //just the selected note and the end
    assertEQ(clip->offset, 0);
}

static MidiSequencerPtr makeSongPut8NotesOnClip()
{
    SqClipboard::clear();
    MidiSequencerPtr seq = makeTest(false);
    assertEQ(seq->context->getTrack()->size(), 9);
    seq->editor->selectAll();
    seq->context->getTrack()->assertValid();

    assert(!SqClipboard::getTrackData());
    seq->editor->copy();
    return seq;
}

static void testCopy2()
{
    printf("testCopy\n");

    MidiSequencerPtr seq = makeSongPut8NotesOnClip();
    assertEQ(seq->context->getTrack()->size(), 9);
    auto clip = SqClipboard::getTrackData();
    assert(clip);
    clip->track->assertValid();
    assertEQ(clip->track->size(), 9);       //just the selected note and the end
    assertEQ(clip->offset, 0);
}

static void testPaste1()
{
    MidiSequencerPtr seq = makeSongPut8NotesOnClip();
    assert(!seq->undo->canUndo());
    seq->selection->clear();   
    seq->editor->paste();
    seq->assertValid();
    assertEQ(seq->context->getTrack()->size(), 9 + 8);
    assert(seq->undo->canUndo());
}

static void testPasteNothingShouldDoNothing()
{
    MidiSequencerPtr seq = makeSongPut8NotesOnClip();
    SqClipboard::clear();
    seq->editor->paste();
    assert(!seq->undo->canUndo());
}

static void testPasteOntoSelection()
{
    MidiSequencerPtr seq = makeSongPut8NotesOnClip();
    assert(!seq->undo->canUndo());
    seq->editor->selectAll();
    seq->editor->paste();
    seq->assertValid();

    // pasting a whole track onto itself should leave no change
    assertEQ(seq->context->getTrack()->size(), 9);
    assert(seq->undo->canUndo());
}

static void testPasteTimeSub(float pasteTime)
{
    printf("----------- testPasteTimeSub\n");
    todo: have sequencer make make a valid context (with two octave range)
    // Make a song with a single note at 1.23
    auto song = MidiSong::MidiSong::makeTest(MidiTrack::TestContent::oneNote123, _trackNumber);
    MidiSequencerPtr seq = MidiSequencer::make(song);
    seq->context->setTrackNumber(_trackNumber);
    seq->assertValid();
    MidiLocker l(seq->song->lock);

    printf("vp = %f, %f\n", seq->context->pitchLow(), seq->context->pitchHi());

    MidiNoteEventPtr note = seq->context->getTrack()->getFirstNote();

    // Set the cursor to be on the one note, and select it
    seq->context->setCursorTime(3);     // we don't use cursor here, so set it crazy
    seq->selection->select(note);

    // copy one note at zero-relative time to clip
    seq->editor->copy();

    assert(SqClipboard::getTrackData());
    assertEQ(SqClipboard::getTrackData()->track->size(), 2);
    float offset = SqClipboard::getTrackData()->offset;
    assertEQ(offset, 1.23f);
    
    // clear out the track
    seq->context->getTrack()->deleteEvent(*note);
    assertEQ(seq->context->getTrack()->size(), 1);

    // now paste at pasteTime, with nothing selected in dest
    seq->context->setCursorTime(pasteTime);
    seq->selection->clear();
    seq->editor->paste();
    seq->assertValid();

    // note should be at zero time
    MidiNoteEventPtr first = seq->context->getTrack()->getFirstNote();
    assert(first);
    assertEQ(first->startTime, pasteTime);

}

static void testPasteTime1()
{
    testPasteTimeSub(0);
}

static void testPasteTime2()
{
    testPasteTimeSub(.3f);
}

static void testPasteTime3()
{
    testPasteTimeSub(4567.89f);
}

static void testMidiEditorCCPSub(int tk)
{
    _trackNumber = tk;
    testCopy1();
    testCopy2();

    testPaste1();
    testPasteNothingShouldDoNothing();
    testPasteOntoSelection();
    testPasteTime1();
    testPasteTime2();
    testPasteTime3();
    // need to test pasting one note onto itself
}


void testMidiEditorCCP()
{
    testMidiEditorCCPSub(0);
    testMidiEditorCCPSub(2);
}