
#include "asserts.h"
#include "MidiEditor.h"
#include "MidiEvent.h"
#include "MidiSelectionModel.h"
#include "MidiSequencer.h"
#include "MidiTrack.h"
#include "MidiSong.h"

MidiSequencerPtr makeTest(bool empty = false)
{
   
    MidiSongPtr song = empty ?
        MidiSong::makeTestEmpty() :
        MidiSong::makeTest1();
    MidiSequencerPtr seq = std::make_shared<MidiSequencer>(song);
    return seq;
}

// from a null selection, select next
static void test1()
{
    MidiSequencerPtr seq = makeTest();
    seq->editor->selectNextNote();
    assertEQ(seq->selection->size(), 1);     // should be one note selected


    MidiEventPtr firstEvent = seq->song->getTrack(0)->begin()->second;
    assert(seq->selection->isSelected(firstEvent));
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


// from a null selection, select next in null track
static void test3()
{
    MidiSequencerPtr seq = makeTest(true);
    seq->editor->selectNextNote();
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

void testMidiEditor()
{
    test1();
    test2();
    test3();
    test4();
}