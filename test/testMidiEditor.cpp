
#include "asserts.h"
#include "MidiEditor.h"
#include "MidiEvent.h"
#include "MidiSelectionModel.h"
#include "MidiSequencer.h"
#include "MidiTrack.h"
#include "MidiViewport.h"
#include "MidiSong.h"

// sequencer factory - helper function
MidiSequencerPtr makeTest(bool empty = false)
{
    MidiSongPtr song = empty ?
        MidiSong::makeTestEmpty() :
        MidiSong::makeTest1();
    MidiSequencerPtr sequencer = std::make_shared<MidiSequencer>(song);


    sequencer->context->viewport->startTime = 0;
    sequencer->context->viewport->endTime =
    sequencer->context->viewport->startTime + 8;
    sequencer->context->viewport->pitchLow = PitchUtils::pitchToCV(3, 0);
    sequencer->context->viewport->pitchHi = PitchUtils::pitchToCV(5, 0);

    sequencer->context->viewport->assertValid();
    return sequencer;
}

static bool cursorOnSelection(MidiSequencerPtr seq)
{
    if (seq->selection->empty()) {
        return true;
    }

    assert(seq->selection->size() == 1);    // haven't done multi yet
    MidiEventPtr sel = *seq->selection->begin();
    MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(sel);
    assert(note);

    // for now, do exact match
    if ((note->startTime == seq->context->cursorTime) &&
        (note->pitchCV == seq->context->cursorPitch)) {
        return true;
    }
    return false;
}

// from a null selection, select next
static void test1()
{
    MidiSequencerPtr seq = makeTest();
    seq->editor->selectNextNote();
    assertEQ(seq->selection->size(), 1);     // should be one note selected

    // note should be first one
    MidiEventPtr firstEvent = seq->song->getTrack(0)->begin()->second;
    assert(seq->selection->isSelected(firstEvent));
    assert(cursorOnSelection(seq));
}

// from a null selection, select previous. should select last note
static void test1b()
{
    MidiSequencerPtr seq = makeTest();
    seq->editor->selectPrevNote();
    assertEQ(seq->selection->size(), 1);     // should be one note selected

    // note should be last one
    auto it = seq->song->getTrack(0)->end();
    it--;
    it--;
    MidiEventPtr lastEvent = it->second;

    assert(lastEvent->type == MidiEvent::Type::Note);
    assert(seq->selection->isSelected(lastEvent));
    assert(cursorOnSelection(seq));
}

// from a non-null selection, select next
static void test2()
{
    MidiSequencerPtr seq = makeTest();
    seq->editor->selectNextNote();
    assertEQ(seq->selection->size(), 1);     // should be one note selected

    MidiEventPtr firstEvent = seq->song->getTrack(0)->begin()->second;
    assert(seq->selection->isSelected(firstEvent));

    // Above is just test1, so now first event selected
    seq->editor->selectNextNote();
    assertEQ(seq->selection->size(), 1);     // should be one note selected

    auto iter = seq->song->getTrack(0)->begin();
    ++iter;
    MidiEventPtr secondEvent = iter->second;
    assert(seq->selection->isSelected(secondEvent));
}

// from a non-null selection, select previous
static void test2b()
{
    MidiSequencerPtr seq = makeTest();

    // Select the second note in the Seq
    seq->editor->selectNextNote();
    seq->editor->selectNextNote();
    assertEQ(seq->selection->size(), 1);

    // Verify that second note is selected
    auto iter = seq->song->getTrack(0)->begin();
    ++iter;
    MidiEventPtr secondEvent = iter->second;
    assert(seq->selection->isSelected(secondEvent));

    // Above is just test1, so now second event selected
    seq->editor->selectPrevNote();
    assertEQ(seq->selection->size(), 1);     // should be one note selected

    iter = seq->song->getTrack(0)->begin();
    MidiEventPtr firstEvent = iter->second;
    assert(seq->selection->isSelected(firstEvent));
}

// from a null selection, select next in null track
static void test3()
{
    MidiSequencerPtr seq = makeTest(true);
    seq->editor->selectNextNote();
    assertEQ(seq->selection->size(), 0);     // should be nothing selected
    assert(seq->selection->empty());
}

// from a null selection, select previous in null track
static void test3b()
{
    MidiSequencerPtr seq = makeTest(true);
    seq->editor->selectPrevNote();
    assertEQ(seq->selection->size(), 0);     // should be nothing selected
    assert(seq->selection->empty());
}

// select one after another until end
static void test4()
{
    MidiSequencerPtr seq = makeTest();
    int notes = 0;
    for (bool done = false; !done; ) {
        seq->editor->selectNextNote();
        if (seq->selection->empty()) {
            done = true;
        } else {
            ++notes;
        }

    }
    assertEQ(notes, 8);
}

static void testPrev1()
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->selectNextNote();          // now first is selected
    assert(!seq->selection->empty());

    seq->editor->selectPrevNote();
    assert(seq->selection->empty());
}

static void testTrans1()
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->selectNextNote();          // now first is selected

    MidiEventPtr firstEvent = seq->song->getTrack(0)->begin()->second;
    MidiNoteEventPtr firstNote = safe_cast<MidiNoteEvent>(firstEvent);
    const float p0 = firstNote->pitchCV;
    seq->editor->transpose(1.0f / 12.f);
    const float p1 = firstNote->pitchCV;
    assertClose(p1 - p0, 1.f / 12.f, .000001);
}

// transpose multi
static void testTrans2()
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->selectNextNote();          // now first is selected

    MidiEventPtr firstEvent = seq->song->getTrack(0)->begin()->second;
    MidiNoteEventPtr firstNote = safe_cast<MidiNoteEvent>(firstEvent);
    const float p0 = firstNote->pitchCV;

    seq->editor->transpose(1.0f / 12.f);
    const float p1 = firstNote->pitchCV;
    assertClose(p1 - p0, 1.f / 12.f, .000001);
}

static void testCursor1()
{
    MidiSequencerPtr seq = makeTest(false);
    assertEQ(seq->context->cursorTime, 0);
    assertEQ(seq->context->cursorPitch, 0)
}

static void testCursor2()
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->advanceCursor(false, 1);
    assertEQ(seq->context->cursorTime, 1.f / 4.f);

    seq->editor->advanceCursor(false, -4);
    assertEQ(seq->context->cursorTime, 0);
}

static void testCursor3()
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->selectNextNote();

    // Select first note to put cursor in it
    assertEQ(seq->context->cursorTime, 0);
    MidiNoteEvent note;
    note.setPitch(3, 0);
    assertEQ(seq->context->cursorPitch, note.pitchCV);

    // Now advance a 1/4 note
    seq->editor->advanceCursor(false, 4);
    assertEQ(seq->context->cursorTime, 1.f);
    assert(seq->selection->empty());
}


// move multiple times in two directions
static void testCursor4()
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->selectNextNote();

    // Select first note to put cursor in it
    assertEQ(seq->context->cursorTime, 0);
    MidiNoteEvent note;
    note.setPitch(3, 0);
    assertEQ(seq->context->cursorPitch, note.pitchCV);

    // Now advance up 3
    seq->editor->changeCursorPitch(1);
    seq->editor->changeCursorPitch(1);
    seq->editor->changeCursorPitch(1);

    for (int i = 0; i < 12; ++i) {
        seq->editor->advanceCursor(false, 1);
    }

    assert(!seq->selection->empty());
}

// just past end
static void testCursor5()
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->selectNextNote();

    // Select first note to put cursor in it
    assertEQ(seq->context->cursorTime, 0);
    MidiNoteEvent note;
    note.setPitch(3, 0);
    assertEQ(seq->context->cursorPitch, note.pitchCV);

    // Now advance two units right, to end of note
    seq->editor->advanceCursor(false, 1);
    seq->editor->advanceCursor(false, 1);

    assert(seq->selection->empty());
}

void testMidiEditor()
{
    test1();
    test1b();
    test2();
    test2b();
    test3();
    test3b();
    testPrev1();
    test4();

    testTrans1();
    testTrans2();

    testCursor1();
    testCursor2();
    testCursor3();
    testCursor4();
    testCursor5();
}