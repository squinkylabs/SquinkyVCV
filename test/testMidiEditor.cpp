
#include "asserts.h"
#include "MidiEditor.h"
#include "MidiEvent.h"
#include "MidiSelectionModel.h"
#include "MidiSequencer.h"
#include "MidiTrack.h"
#include "MidiSong.h"
#include "TestSettings.h"
#include "TimeUtils.h"

static int _trackNumber = 0;

// sequencer factory - helper function
static MidiSequencerPtr makeTest(bool empty = false)
{
    MidiSongPtr song = empty ?
        MidiSong::MidiSong::makeTest(MidiTrack::TestContent::empty, _trackNumber) :
        MidiSong::MidiSong::makeTest(MidiTrack::TestContent::eightQNotes, _trackNumber);
    MidiSequencerPtr sequencer = MidiSequencer::make(song, std::make_shared<TestSettings>());
   // sequencer->makeEditor();

    sequencer->context->setTrackNumber(_trackNumber);
    sequencer->context->setStartTime(0);
    sequencer->context->setEndTime(
        sequencer->context->startTime() + 8);
    sequencer->context->setPitchLow(PitchUtils::pitchToCV(3, 0));
    sequencer->context->setPitchHi(PitchUtils::pitchToCV(5, 0));


    sequencer->assertValid();
    return sequencer;
}

// transpose one semi
static void testTrans1()
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->selectNextNote();          // now first is selected

    const float firstNotePitch = PitchUtils::pitchToCV(3, 0);
    assertClose(seq->context->cursorPitch(), firstNotePitch, .0001);

    MidiEventPtr firstEvent = seq->context->getTrack()->begin()->second;
    MidiNoteEventPtr firstNote = safe_cast<MidiNoteEvent>(firstEvent);
    const float p0 = firstNote->pitchCV;
    seq->editor->changePitch(1);
    //seq->assertValid();

    // after transpose, need to find first note again.
    firstEvent = seq->context->getTrack()->begin()->second;
    firstNote = safe_cast<MidiNoteEvent>(firstEvent);

    const float p1 = firstNote->pitchCV;
    assertClose(p1 - p0, 1.f / 12.f, .000001);
    const float transposedPitch = PitchUtils::pitchToCV(3, 1);
    assertClose(seq->context->cursorPitch(), transposedPitch, .0001);
    seq->assertValid();
    seq->editor->assertCursorInSelection();
}


static void testTrans3Sub(int semitones)
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->selectNextNote();          // now first is selected

    const float firstNotePitch = PitchUtils::pitchToCV(3, 0);
    assertClose(seq->context->cursorPitch(), firstNotePitch, .0001);

    MidiEventPtr firstEvent = seq->context->getTrack()->begin()->second;
    MidiNoteEventPtr firstNote = safe_cast<MidiNoteEvent>(firstEvent);
    const float p0 = firstNote->pitchCV;
    seq->editor->changePitch(semitones);       // transpose off screen
    seq->assertValid();
    seq->editor->assertCursorInSelection();
}

static void testTrans3()
{
    testTrans3Sub(50);
}

static void testTransHuge()
{
    testTrans3Sub(300);
    testTrans3Sub(-300);
}

static void testShiftTime1()
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->selectNextNote();          // now first is selected


    MidiNoteEventPtr firstNote = seq->context->getTrack()->getFirstNote();

    const float s0 = firstNote->startTime;
    seq->editor->changeStartTime(false, 1);     // delay one unit (1/16 6h)

    firstNote = seq->context->getTrack()->getFirstNote();
    const float s1 = firstNote->startTime;
    assertClose(s1 - s0, 1.f / 4.f, .000001);

    seq->editor->changeStartTime(false, -50);
    firstNote = seq->context->getTrack()->getFirstNote();
    const float s2 = firstNote->startTime;
    assertEQ(s2, 0);
    seq->assertValid();
    seq->editor->assertCursorInSelection();
}

static void testShiftTimex(int units)
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->selectNextNote();          // now first is selected

    MidiEventPtr firstEvent = seq->context->getTrack()->begin()->second;
    MidiNoteEventPtr firstNote = safe_cast<MidiNoteEvent>(firstEvent);
    const float s0 = firstNote->startTime;
    seq->editor->changeStartTime(false, units);     // delay n units
    seq->assertValid();

    assertEQ(seq->selection->size(), 1);
    seq->editor->assertCursorInSelection();
}

static void testShiftTime2()
{
    testShiftTimex(20);
}


static void testShiftTime3()
{
    testShiftTimex(50);
}

// TODO: make this work with negative shift.
static void testShiftTimeTicks(int howMany)
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->selectNextNote();          // now first is selected
    seq->editor->selectNextNote();          // now second

    MidiNoteEventPtr secondNote = seq->context->getTrack()->getSecondNote();


    const float s0 = secondNote->startTime;
    seq->editor->changeStartTime(true, howMany);     // delay n "ticks" 

    // find second again.
    secondNote = seq->context->getTrack()->getSecondNote();

    const float s1 = secondNote->startTime;
    assertClose(s1 - s0, howMany / 16.f, .000001);
}

static void testShiftTimeTicks0()
{
    testShiftTimeTicks(1);
}

static void testShiftTimeTicks1()
{
    testShiftTimeTicks(-1);
}

static void testChangeDuration1()
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->selectNextNote();          // now first is selected

    MidiNoteEventPtr firstNote = seq->context->getTrack()->getFirstNote();
    const float d0 = firstNote->duration;
    seq->editor->changeDuration(false, 1);     // lengthen one unit

    firstNote = seq->context->getTrack()->getFirstNote();
    const float d1 = firstNote->duration;
    assertClose(d1 - d0, 1.f / 4.f, .000001);
    seq->assertValid();

    // try to make negative, should not go below 1
    seq->editor->changeDuration(false, -50);
    firstNote = seq->context->getTrack()->getFirstNote();
    const float d2 = firstNote->duration;
    assertGT(d2, 0);
    seq->assertValid();
}

static void testChangeDurationTicks()
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->selectNextNote();          // now first is selected

    MidiNoteEventPtr firstNote = seq->context->getTrack()->getFirstNote();
    const float d0 = firstNote->duration;
    seq->editor->changeDuration(true, 1);     // lengthen one tick

    firstNote = seq->context->getTrack()->getFirstNote();
    const float d1 = firstNote->duration;
    assertClose(d1 - d0, 1.f / 16.f, .000001);
    seq->assertValid();

    // try to make negative, should not go below 1
    seq->editor->changeDuration(true, -50);
    firstNote = seq->context->getTrack()->getFirstNote();
    const float d2 = firstNote->duration;
    assertGT(d2, 0);
    seq->assertValid();
}

// transpose multi
static void testTrans2()
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->selectNextNote();          // now first is selected

    MidiEventPtr firstEvent = seq->context->getTrack()->begin()->second;
    MidiNoteEventPtr firstNote = safe_cast<MidiNoteEvent>(firstEvent);
    const float p0 = firstNote->pitchCV;

    seq->editor->changePitch(1);
    firstEvent = seq->context->getTrack()->begin()->second;
    firstNote = safe_cast<MidiNoteEvent>(firstEvent);


    const float p1 = firstNote->pitchCV;
    assertClose(p1 - p0, 1.f / 12.f, .000001);
    seq->assertValid();

    assert(seq->undo->canUndo());
    seq->undo->undo(seq);
    MidiNoteEventPtr firstNoteAfterUndo = safe_cast<MidiNoteEvent>(seq->context->getTrack()->begin()->second);
    const float p3 = firstNoteAfterUndo->pitchCV;
    assertClose(p3, p0, .000001);
    seq->undo->redo(seq);
    MidiNoteEventPtr firstNoteAfterRedo = safe_cast<MidiNoteEvent>(seq->context->getTrack()->begin()->second);
    const float p4 = firstNoteAfterRedo->pitchCV;
    assertClose(p4, p1, .000001);


}

static void testCursor1()
{
    MidiSequencerPtr seq = makeTest(false);
    assertEQ(seq->context->cursorTime(), 0);
    assertEQ(seq->context->cursorPitch(), 0)
        assertEQ(seq->context->startTime(), 0);
}

static void testCursor2()
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->advanceCursor(false, 1);
    assertEQ(seq->context->cursorTime(), 1.f / 4.f);

    seq->editor->advanceCursor(false, -4);
    assertEQ(seq->context->cursorTime(), 0);
    assertEQ(seq->context->startTime(), 0);
}

static void testCursor3()
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->selectNextNote();

    // Select first note to put cursor in it
    assertEQ(seq->context->cursorTime(), 0);
    MidiNoteEvent note;
    note.setPitch(3, 0);
    assertEQ(seq->context->cursorPitch(), note.pitchCV);

    // Now advance a 1/4 note
    seq->editor->advanceCursor(false, 4);
    assertEQ(seq->context->cursorTime(), 1.f);
    assert(seq->selection->empty());
    assertEQ(seq->context->startTime(), 0);

}

// move multiple times in two directions
static void testCursor4()
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->selectNextNote();

    // Select first note to put cursor in it
    assertEQ(seq->context->cursorTime(), 0);
    MidiNoteEvent note;
    note.setPitch(3, 0);
    assertEQ(seq->context->cursorPitch(), note.pitchCV);

    // Now advance up 3
    seq->editor->changeCursorPitch(1);
    seq->editor->changeCursorPitch(1);
    seq->editor->changeCursorPitch(1);

    for (int i = 0; i < 12; ++i) {
        seq->editor->advanceCursor(false, 1);
    }

    assert(!seq->selection->empty());
    assertEQ(seq->context->startTime(), 0);
}


// move up to scroll viewport
static void testCursor4b()
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->selectNextNote();

    // Select first note to put cursor in it
    assertEQ(seq->context->cursorTime(), 0);
    MidiNoteEvent note;
    note.setPitch(3, 0);
    assertEQ(seq->context->cursorPitch(), note.pitchCV);

    // Now advance up 3 octaves
    seq->editor->changeCursorPitch(3 * 12);

    assert(seq->selection->empty());
    seq->assertValid();
}


// just past end of note
static void testCursor5()
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->selectNextNote();

    // Select first note to put cursor in it
    assertEQ(seq->context->cursorTime(), 0);
    MidiNoteEvent note;
    note.setPitch(3, 0);
    assertEQ(seq->context->cursorPitch(), note.pitchCV);

    // Now advance two units right, to end of note
    seq->editor->advanceCursor(false, 1);
    seq->editor->advanceCursor(false, 1);

    assert(seq->selection->empty());
    assertEQ(seq->context->startTime(), 0);
}

// move past the end of the second bar
static void testCursor6()
{
    MidiSequencerPtr seq = makeTest(false);

    assertEQ(seq->context->startTime(), 0);
    seq->assertValid();

    // go up two bars and 1/16
    seq->editor->advanceCursor(false, 16 * 2 + 1);

    // bar 2 should be new start time
    assertEQ(seq->context->startTime(), TimeUtils::bar2time(2));
    assertEQ(seq->context->endTime(), TimeUtils::bar2time(4));

}

static void testInsertSub(int advancUnits)
{
    MidiSequencerPtr seq = makeTest(true);
    assert(seq->selection->empty());
    const int initialSize = seq->context->getTrack()->size();

    seq->editor->advanceCursor(false, advancUnits);       // move up a half note
    float pitch = seq->context->cursorPitch();

    seq->editor->insertNote();

    auto it = seq->context->getTrack()->begin();
    assert(it != seq->context->getTrack()->end());
    MidiEventPtr ev = it->second;
    MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(ev);
    assert(note);

    assertEQ(note->pitchCV, pitch);
    assertEQ(seq->selection->size(), 1);

    assert(seq->selection->isSelected(note));
    seq->assertValid();
    const int insertSize = seq->context->getTrack()->size();
    assertGT(insertSize, initialSize);

#if 0
    assert(seq->undo->canUndo());
    seq->undo->undo();
    const int undoSize = seq->context->getTrack()->size();
    assert(undoSize == initialSize);
#endif
}

static void testInsert()
{
    testInsertSub(8);
}

static void testInsert2()
{
    testInsertSub(34);      //middle of second bar
}

static float getDuration(MidiEditor::Durations dur)
{
    float ret = 0;
    switch (dur) {
        case MidiEditor::Durations::Whole:
            ret = 4;
            break;
        case MidiEditor::Durations::Half:
            ret = 2;
            break;
        case MidiEditor::Durations::Quarter:
            ret = 1;
            break;
        case MidiEditor::Durations::Eighth:
            ret = .5;
            break;
        case MidiEditor::Durations::Sixteenth:
            ret = .25f;
            break;
        default:
            assert(false);
    }
    return ret;
}

static void testInsertPresetNote(MidiEditor::Durations dur, bool advance, float articulation)
{

    MidiSequencerPtr seq = makeTest(true);
    assert(seq->selection->empty());
    const int initialSize = seq->context->getTrack()->size();

    auto s = seq->context->settings();
    TestSettings* ts = dynamic_cast<TestSettings*>(s.get());
    assert(ts);
    ts->_articulation = articulation;

    assertEQ(seq->context->cursorTime(), 0);
    float pitch = seq->context->cursorPitch();

    seq->editor->insertPresetNote(dur, advance);

    auto it = seq->context->getTrack()->begin();
    assert(it != seq->context->getTrack()->end());
    MidiEventPtr ev = it->second;
    MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(ev);
    assert(note);

    assertEQ(note->pitchCV, pitch);
    const int expectedSelection = advance ? 0 : 1;
    assertEQ(seq->selection->size(), expectedSelection);

    const float expectedDuration = getDuration(dur) * articulation;
    assertEQ(note->duration, expectedDuration);

    const bool expectSelected = advance ? false : true;
    assertEQ(seq->selection->isSelected(note), expectSelected);
    seq->assertValid();
    const int insertSize = seq->context->getTrack()->size();
    assertGT(insertSize, initialSize);

    const float expectedCursorTime = advance ? getDuration(dur) : 0;
    assertEQ(seq->context->cursorTime(), expectedCursorTime);
}


static void testInsertPresetNotes()
{
    testInsertPresetNote(MidiEditor::Durations::Quarter, false, .5f);
    testInsertPresetNote(MidiEditor::Durations::Half, true, .85f);
    testInsertPresetNote(MidiEditor::Durations::Whole, true, .1f);
    testInsertPresetNote(MidiEditor::Durations::Eighth, false, 1.01f);
    testInsertPresetNote(MidiEditor::Durations::Sixteenth, true, .2f);
}

static void testDelete()
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->selectNextNote();

    auto it = seq->context->getTrack()->begin();
    assert(it != seq->context->getTrack()->end());
    MidiEventPtr ev = it->second;
    MidiNoteEventPtr firstNote = safe_cast<MidiNoteEvent>(ev);
    assert(firstNote);
    assertEQ(firstNote->startTime, 0);

    assertEQ(seq->selection->size(), 1);
    seq->editor->deleteNote();
    assert(seq->selection->empty());

    it = seq->context->getTrack()->begin();
    assert(it != seq->context->getTrack()->end());
    ev = it->second;
    MidiNoteEventPtr secondNote = safe_cast<MidiNoteEvent>(ev);
    assert(secondNote);
    assertEQ(secondNote->startTime, 1.f);
    seq->assertValid();
}

// delete a note with undo/redo
static void testDelete2()
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->selectNextNote();
    const int trackSizeBefore = seq->context->getTrack()->size();
    seq->editor->deleteNote();
    const int trackSizeAfter = seq->context->getTrack()->size();
    assertLT(trackSizeAfter, trackSizeBefore);

    assert(seq->undo->canUndo());
    seq->undo->undo(seq);
    const int trackSizeAfterUndo = seq->context->getTrack()->size();
    assertEQ(trackSizeAfterUndo, trackSizeBefore);
}

void testMidiEditorSub(int trackNumber)
{
    _trackNumber = trackNumber;



    testTrans1();
    testShiftTime1();
    testShiftTime2();
    testShiftTime3();
    testShiftTimeTicks0();
    testShiftTimeTicks1();

    testChangeDuration1();
    testChangeDurationTicks();

    testTrans2();
    testTrans3();
    testTransHuge();

    testCursor1();
    testCursor2();
    testCursor3();
    testCursor4();
    testCursor4b();
    testCursor5();
    testCursor6();

    testInsert();
    testInsert2();
    testDelete();
    testDelete2();
    testInsertPresetNotes();

}

void testMidiEditor()
{
    testMidiEditorSub(0);
    testMidiEditorSub(2);
}