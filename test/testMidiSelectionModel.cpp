#include "MidiLock.h"
#include "MidiSequencer.h"
#include "TestAuditionHost.h"

#include <memory>

static void testExtendSelection()
{
    auto a = std::make_shared<TestAuditionHost>();
    // put in one, then extend to second one
    MidiSelectionModel sel(a);

    assert(sel.empty());
    assertEQ(sel.size(), 0);
    assert(!sel.isAllSelected());

    MidiNoteEventPtr note1 = std::make_shared<MidiNoteEvent>();
    MidiNoteEventPtr note2 = std::make_shared<MidiNoteEvent>();
    note1->startTime = 1;
    note1->pitchCV = 1.1f;
    note2->startTime = 2;
    note2->pitchCV = 2.1f;

    sel.select(note2);
    sel.extendSelection(note1);

    assert(!sel.empty());
    assertEQ(sel.size(), 2);
    assert(!sel.isAllSelected());

    // should find the events
    bool found1 = false;
    bool found2 = false;
    for (auto it : sel) {
        if (it == note1) {
            found1 = true;
        }
        if (it == note2) {
            found2 = true;
        };
    }
    assert(found1);
    assert(found2);
}


static void testAddSelectionSameNote()
{
    auto a = std::make_shared<TestAuditionHost>();

    MidiSelectionModel sel(a);
    MidiNoteEventPtr note1 = std::make_shared<MidiNoteEvent>();
    MidiNoteEventPtr note2 = std::make_shared<MidiNoteEvent>();

    assert(*note1 == *note2);

    sel.select(note1);
    sel.extendSelection(note2);
    assertEQ(sel.size(), 1);
    assert(!sel.isAllSelected());
}

static void testSelectionDeep()
{
    auto a = std::make_shared<TestAuditionHost>();
    MidiSelectionModel selOrig(a);
    MidiNoteEventPtr note1 = std::make_shared<MidiNoteEvent>();
    MidiNoteEventPtr note2 = std::make_shared<MidiNoteEvent>();
    MidiNoteEventPtr note3 = std::make_shared<MidiNoteEvent>();
    note1->startTime = 1;
    note1->pitchCV = 1.1f;
    note2->startTime = 2;
    note2->pitchCV = 2.1f;
    note3->startTime = 3;

    assert(selOrig.empty());
    assertEQ(selOrig.size(), 0);

    selOrig.select(note2);
    selOrig.extendSelection(note1);

    assert(!selOrig.empty());
    assertEQ(selOrig.size(), 2);

    MidiSelectionModelPtr sel = selOrig.clone();
    assert(sel);
    assert(sel->size() == selOrig.size());

    assert(sel->isSelectedDeep(note1));
    assert(sel->isSelectedDeep(note2));
    assert(!sel->isSelectedDeep(note3));
    assert(!sel->isAllSelected());
}

void testSelectionAddTwice()
{
    auto a = std::make_shared<TestAuditionHost>();
    MidiSelectionModel selOrig(a);
    MidiNoteEventPtr note1 = std::make_shared<MidiNoteEvent>();
    MidiNoteEventPtr note2 = std::make_shared<MidiNoteEvent>();
    note1->startTime = 1;
    note1->pitchCV = 1.1f;
    note2->startTime = 2;
    note2->pitchCV = 2.1f;
    

    MidiSelectionModel sel(a);
    sel.extendSelection(note1);
    sel.extendSelection(note2);
    assertEQ(sel.size(), 2);

    sel.extendSelection(note1);
    assertEQ(sel.size(), 2);

    //  clone should be recognized as the same.
    MidiEventPtr cloneNote1 = note1->clone();
    sel.extendSelection(cloneNote1);
    assertEQ(sel.size(), 2);
    assert(!sel.isAllSelected());
}

static void testSelectionSelectAll()
{
    MidiLockPtr lock = std::make_shared<MidiLock>();
    MidiLocker l(lock);
    MidiTrackPtr tk = MidiTrack::makeTest(MidiTrack::TestContent::eightQNotes, lock);
    auto a = std::make_shared<TestAuditionHost>();
    MidiSelectionModel sel(a);

    assert(!sel.isAllSelected());
    sel.selectAll(tk);

    // same number of notes (sel doesn't have end event)
    assertEQ(sel.size(), tk->size() - 1);
    assert(sel.isAllSelected());

    sel.clear();
    assert(!sel.isAllSelected());
}

static void testSelectionSelectAll2()
{
    MidiLockPtr lock = std::make_shared<MidiLock>();
    MidiLocker l(lock);
    MidiTrackPtr tk = MidiTrack::makeTest(MidiTrack::TestContent::eightQNotes, lock);
    auto a = std::make_shared<TestAuditionHost>();
    MidiSelectionModel sel(a);
    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();

    sel.selectAll(tk);
    assert(sel.isAllSelected());
    sel.addToSelection(note, false);
    assert(!sel.isAllSelected());

    sel.selectAll(tk);
    assert(sel.isAllSelected());
    sel.addToSelection(note, true);
    assert(!sel.isAllSelected());

    sel.selectAll(tk);
    assert(sel.isAllSelected());
    sel.select(note);
    assert(!sel.isAllSelected());

    sel.selectAll(tk);
    assert(sel.isAllSelected());
    sel.extendSelection(note);
    assert(!sel.isAllSelected());

    sel.selectAll(tk);
    assert(sel.isAllSelected());
    sel.extendSelection(note);
    sel.removeFromSelection(note);
    assert(!sel.isAllSelected());

}

void testMidiSelectionModel()
{
    testExtendSelection();
      //  testExtendSelection();
    testAddSelectionSameNote();
    testSelectionDeep();
    testSelectionAddTwice();
    testSelectionSelectAll();
    testSelectionSelectAll2();
}