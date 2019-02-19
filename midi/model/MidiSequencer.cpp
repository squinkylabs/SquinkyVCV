
#include "MidiSequencer.h"
#include "MidiEditor.h"
#include "UndoRedoStack.h"

int _mdb = 0;       // global instance counter

MidiSequencer::MidiSequencer(MidiSongPtr sng) :
    selection(std::make_shared<MidiSelectionModel>()),
    song(sng),
    context(std::make_shared<MidiEditorContext>(sng)
    )
{
    undo = std::make_shared<UndoRedoStack>();
    ++_mdb;
}

MidiSequencerPtr MidiSequencer::make(MidiSongPtr song)
{
    MidiSequencerPtr seq(new MidiSequencer(song));
    seq->makeEditor();

    // Find a track to point the edit context at
    bool found = false;
    int maxTk = song->getHighestTrackNumber();
    for (int i = 0; i <= maxTk; ++i) {
        if (song->trackExists(i)) {
            seq->context->setTrackNumber(i);
            found = true;
            break;
        }
    }
    (void) found;
    assert(found);
    seq->assertValid();
    return seq;
}
 

void MidiSequencer::makeEditor()
{
    MidiSequencerPtr seq = shared_from_this();
    editor = std::make_shared<MidiEditor>(seq);
}

MidiSequencer::~MidiSequencer()
{
    --_mdb;
}


void MidiSequencer::assertValid() const
{
    assert(editor);
    assert(undo);
    assert(song);
    assert(context);
    assert(selection);
    context->assertValid();
    song->assertValid(); 
    assertSelectionInTrack();
}

void MidiSequencer::assertSelectionInTrack() const
{
    MidiTrackPtr track = context->getTrack();
    
    for (auto it : *selection) {
        auto foundPtr = track->findEventPointer(it);
        assert(foundPtr != track->end());
        auto x = *foundPtr;
        MidiEventPtrC y = x.second;
    }
}