
#include "asserts.h"
#include "MidiEditor.h"
#include "MidiEvent.h"
#include "MidiSelectionModel.h"
#include "MidiSequencer.h"
#include "MidiTrack.h"
#include "MidiSong.h"

MidiSequencerPtr makeTest()
{
    MidiSongPtr song = MidiSong::makeTest1();
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

void testMidiEditor()
{
    test1();
    test2();
}