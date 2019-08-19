
#include "asserts.h"
#include "MidiEditor.h"
#include "MidiEvent.h"
#include "MidiLock.h"
#include "MidiSelectionModel.h"
#include "MidiSequencer.h"
#include "MidiTrack.h"
#include "MidiSong.h"
#include "MLockTest.h"
#include "SqClipboard.h"
#include "TestAuditionHost.h"
#include "TestSettings.h"
#include "TimeUtils.h"



static int _trackNumber = 0;

// sequencer factory - helper function
static MidiSequencerPtr makeTest(bool empty = false)
{
    MidiSongPtr song = empty ?
        MidiSong::MidiSong::makeTest(MidiTrack::TestContent::empty, _trackNumber) :
        MidiSong::MidiSong::makeTest(MidiTrack::TestContent::eightQNotes, _trackNumber);
    MidiSequencerPtr sequencer = MidiSequencer::make(
        song,
        std::make_shared<TestSettings>(),
        std::make_shared<TestAuditionHost>());
   // sequencer->makeEditor();

    sequencer->context->setTrackNumber(_trackNumber);
    sequencer->context->setStartTime(0);
    sequencer->context->setEndTime(
        sequencer->context->startTime() + 8);
    sequencer->context->setPitchLow(PitchUtils::pitchToCV(3, 0));
    sequencer->context->setPitchHi(PitchUtils::pitchToCV(5, 0));


    sequencer->assertValid();
    sequencer->song->lock->dataModelDirty();
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
    MLockTest l(seq);

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
    MLockTest l(seq);

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

    auto au = seq->selection->_testGetAudition();
    TestAuditionHost* audition = dynamic_cast<TestAuditionHost*>(au.get());
    assert(audition);
    seq->editor->selectNextNote();          // now first is selected

    assertEQ(audition->count(), 1);
    audition->reset();

    MidiNoteEventPtr firstNote = seq->context->getTrack()->getFirstNote();

    const float s0 = firstNote->startTime;
    MLockTest l(seq);

    seq->editor->changeStartTime(false, 1);     // delay one unit (1/16 6h)
    assertEQ(audition->count(), 0);
    audition->reset();

    firstNote = seq->context->getTrack()->getFirstNote();
    const float s1 = firstNote->startTime;
    assertClose(s1 - s0, 1.f / 4.f, .000001);

    seq->editor->changeStartTime(false, -50);
    firstNote = seq->context->getTrack()->getFirstNote();
    const float s2 = firstNote->startTime;
    assertEQ(s2, 0);
    seq->assertValid();
    seq->editor->assertCursorInSelection();

    assertEQ(audition->notes.size(), 0);        // shift in time should not re-audition
}

static void testShiftTimex(int units)
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->selectNextNote();          // now first is selected

    MidiEventPtr firstEvent = seq->context->getTrack()->begin()->second;
    MidiNoteEventPtr firstNote = safe_cast<MidiNoteEvent>(firstEvent);
    const float s0 = firstNote->startTime;
    MLockTest l(seq);

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
    MLockTest l(seq);
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
    MLockTest l(seq);
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

    MLockTest l(seq);
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

static void testCursor2Sub(MidiEditor::Advance adv, float expectedShift)
{
    MidiSequencerPtr seq = makeTest(false);

    auto s = seq->context->settings();
    TestSettings* ts = dynamic_cast<TestSettings*>(s.get());
    assert(ts);
    ts->_quartersInGrid = expectedShift;


    seq->editor->advanceCursor(adv, 1);
    assertEQ(seq->context->cursorTime(), expectedShift);

    seq->editor->advanceCursor(adv, -1);
    assertEQ(seq->context->cursorTime(), 0);
    assertEQ(seq->context->startTime(), 0);
}

static void testCursorAll()
{
    // make a two bar test seq.
    // "end" should be the start of the second bar
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->advanceCursor(MidiEditor::Advance::All, 1);
    assertEQ(seq->context->cursorTime(), 4);
}

static void testCursor2()
{
    testCursor2Sub(MidiEditor::Advance::GridUnit, 1.f / 4.f);
    testCursor2Sub(MidiEditor::Advance::GridUnit, 1.f);
    testCursor2Sub(MidiEditor::Advance::Beat, 1.f);
    testCursor2Sub(MidiEditor::Advance::Tick, 4.f / 64.f);
    testCursor2Sub(MidiEditor::Advance::Measure, 4.f);
    testCursorAll();
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
    seq->editor->advanceCursor(MidiEditor::Advance::Beat, 1);
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


    auto s = seq->context->settings();
    TestSettings* ts = dynamic_cast<TestSettings*>(s.get());
    assert(ts);
    ts->_quartersInGrid = .25;              // set grid to 1/16

    // Now advance up 3 1/16
    seq->editor->changeCursorPitch(1);
    seq->editor->changeCursorPitch(1);
    seq->editor->changeCursorPitch(1);

    for (int i = 0; i < 12; ++i) {
        seq->editor->advanceCursor(MidiEditor::Advance::GridUnit, 1);
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

    auto s = seq->context->settings();
    TestSettings* ts = dynamic_cast<TestSettings*>(s.get());
    assert(ts);
    ts->_quartersInGrid = .25;

    // Now advance two 1/16 units right, to end of note
    seq->editor->advanceCursor(MidiEditor::Advance::GridUnit, 1);
    seq->editor->advanceCursor(MidiEditor::Advance::GridUnit, 1);

    assert(seq->selection->empty());
    assertEQ(seq->context->startTime(), 0);
}

// move past the end of the second bar
static void testCursor6()
{
    MidiSequencerPtr seq = makeTest(false);

    assertEQ(seq->context->startTime(), 0);
    seq->assertValid();

    auto s = seq->context->settings();
    TestSettings* ts = dynamic_cast<TestSettings*>(s.get());
    assert(ts);
    ts->_quartersInGrid = .25;

    // go up two bars and 1/16
    seq->editor->advanceCursor(MidiEditor::Advance::GridUnit, 16 * 2 + 1);

    // bar 2 should be new start time
    assertEQ(seq->context->startTime(), TimeUtils::bar2time(2));
    assertEQ(seq->context->endTime(), TimeUtils::bar2time(4));
}


static void testCursorNonQuantSnaps()
{ 
    MidiEditor::Advance adv = MidiEditor::Advance::GridUnit;
    MidiSequencerPtr seq = makeTest(false);

    auto s = seq->context->settings();
    TestSettings* ts = dynamic_cast<TestSettings*>(s.get());
    assert(ts);
    ts->_quartersInGrid = 1;            // set test grid to a quarter

    seq->context->setCursorTime(.123f);

    seq->editor->advanceCursor(adv, 1);
    assertEQ(seq->context->cursorTime(), 1);        // expect to go to the next grid point.
}

static void testInsertSub(int advancUnitsBeforeInsert, bool advanceAfter, float testGridSize)
{
    MidiSequencerPtr seq = makeTest(true);
    assert(seq->selection->empty());
    const int initialSize = seq->context->getTrack()->size();

    auto s = seq->context->settings();
    TestSettings* ts = dynamic_cast<TestSettings*>(s.get());
    assert(ts);
    ts->_quartersInGrid = testGridSize;

    seq->editor->advanceCursor(MidiEditor::Advance::Tick, advancUnitsBeforeInsert);
    
    // advance cursor may have dirtied the lock. let's clear
    seq->context->getTrack()->lock->dataModelDirty();

    float pitch = seq->context->cursorPitch();

    const float dur = s->getQuarterNotesInGrid();

    MLockTest l(seq);
    
    seq->editor->insertNote(dur, advanceAfter);

    auto it = seq->context->getTrack()->begin();
    assert(it != seq->context->getTrack()->end());
    MidiEventPtr ev = it->second;
    MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(ev);
    assert(note);

    assertEQ(note->pitchCV, pitch);
  
    assertEQ(note->duration, testGridSize);

    float expectedCursorTime = note->startTime + ((advanceAfter) ? testGridSize : 0);
    assertEQ(expectedCursorTime, seq->context->cursorTime());

    if (!advanceAfter) {     // these asserts don't always make sense here
        assertEQ(seq->selection->size(), 1);
        assert(seq->selection->isSelected(note));
    }
  
    seq->assertValid();
    const int insertSize = seq->context->getTrack()->size();
    assertGT(insertSize, initialSize);

#if 1
    assert(seq->undo->canUndo());
    seq->undo->undo(seq);
    const int undoSize = seq->context->getTrack()->size();
    assert(undoSize == initialSize);
#endif
}

static void testInsert()
{
    // static void testInsertSub(int advancUnitsBeforeInsert, bool advanceAfter, float testGridSize)
    testInsertSub(8, false, 1.f / 4.f);         // old default - 16th, no advance
    testInsertSub(34, false, 1.f / 4.f);      //middle of second bar

    testInsertSub(8, false, 1);
    testInsertSub(8, true, 1);

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
    MLockTest l(seq);

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
    MLockTest l(seq);

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
    MLockTest l(seq);
    seq->editor->deleteNote();
    const int trackSizeAfter = seq->context->getTrack()->size();
    assertLT(trackSizeAfter, trackSizeBefore);

    assert(seq->undo->canUndo());
    seq->undo->undo(seq);
    const int trackSizeAfterUndo = seq->context->getTrack()->size();
    assertEQ(trackSizeAfterUndo, trackSizeBefore);
}


static void testChangeTrackLength(bool snap)
{
    MidiSequencerPtr seq = makeTest(false);

    auto s = seq->context->settings();
    TestSettings* ts = dynamic_cast<TestSettings*>(s.get());
    assert(ts);
    ts->_quartersInGrid = 1;        // set to quarter note grid
    ts->_snapToGrid = snap;

    MLockTest l(seq);
    const float initialLength = seq->context->getTrack()->getLength();

    seq->editor->advanceCursorToTime(41.356f, false);
    seq->editor->changeTrackLength();
    seq->assertValid();

    assertEQ(seq->context->getTrack()->getLength(), 41);
    seq->undo->undo(seq);
    assertEQ(seq->context->getTrack()->getLength(), initialLength);
    seq->undo->redo(seq);
    assertEQ(seq->context->getTrack()->getLength(), 41);

    seq->assertValid();
}

static void testChangeTrackLength()
{
    testChangeTrackLength(true);
}

static void testChangeTrackLengthNoSnap()
{
    testChangeTrackLength(false);
}

static void testCut()
{
    MidiSequencerPtr seq = makeTest(false);
  
    const float origLen = seq->context->getTrack()->getLength();
    const int origSize = seq->context->getTrack()->size();
    seq->editor->selectAll();
    MLockTest l(seq);

    seq->editor->cut();
  
    seq->assertValid();

    assertEQ(1, seq->context->getTrack()->size());

    seq->undo->undo(seq);
    assertEQ(origSize, seq->context->getTrack()->size());

    auto clipData = SqClipboard::getTrackData();
    assertEQ(clipData->offset, 0);
    MidiTrackPtr tk = clipData->track;

    assertEQ(tk->size(), origSize);
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
    testCursorNonQuantSnaps();

    testInsert();
    testDelete();
    testDelete2();
    testInsertPresetNotes();

    testChangeTrackLength();
    testChangeTrackLengthNoSnap();
    testCut();
}

void testMidiEditor()
{
    testMidiEditorSub(0);
    testMidiEditorSub(2);
}