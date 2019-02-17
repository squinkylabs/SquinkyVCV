
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
    assert(found);
    song->_e();
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
    song->_e();
    MidiTrackPtr track = context->getTrack();
    
    for (auto it : *selection) {
#if 1
        
        auto foundPtr = track->findEventPointer(it);
        assert(foundPtr != track->end());
        auto x = *foundPtr;
        MidiEventPtrC y = x.second;

       // MidiEventPtrC x = track->findEventPointer(it);;
#else
       MidiEventPtrC foundPtr = track->findEventPointer(it)->second;
       assert(foundPtr);
#endif
       
       
    }
}