
#include "MidiEvent.h"
#include "MidiSelectionModel.h"
#include "MidiSequencer.h"
#include "MidiSong.h"
#include "MidiTrack.h"

static void testSelectionModel1()
{
    MidiSelectionModelPtr sel = std::make_shared<MidiSelectionModel>();
    MidiSongPtr song = MidiSong::makeTest1();
    MidiEventPtr evt = song->getTrack(0)->begin()->second;
    assert(evt);
    sel->select(evt);

    int ct = 0;

    for (auto it = sel->begin(); it != sel->end(); ++it) {
        ++ct;
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(*it);
        assert(note);
    }
    assertEQ(ct, 1);
}


static void testMidiSequencer1()
{
    MidiSongPtr song = MidiSong::makeTest1();
    MidiSequencerPtr seq = std::make_shared<MidiSequencer>(song);

    assert(seq->selection);
    auto sel = seq->selection;
    assert(sel->begin() == sel->end());

    assert( seq->editor);
}


void testMidiControllers()
{
    testSelectionModel1();
    testMidiSequencer1();
}
