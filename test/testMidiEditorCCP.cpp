
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
    fflush(stdout);
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

static void testMidiEditorCCPSub(int tk)
{
    _trackNumber = tk;
    testCopy1();
    testCopy2();

    testPaste1();
    testPasteNothingShouldDoNothing();
    testPasteOntoSelection();
}


void testMidiEditorCCP()
{
    testMidiEditorCCPSub(0);
    testMidiEditorCCPSub(2);
}