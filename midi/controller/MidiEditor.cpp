
#include <assert.h>
#include "MidiEditor.h"
#include "MidiSelectionModel.h"
#include "MidiSong.h"
#include "MidiTrack.h"

extern int _mdb;

MidiEditor::MidiEditor(std::shared_ptr<MidiSong> sng, std::shared_ptr<MidiSelectionModel> sel) :
    song(sng), selection(sel)
{
    _mdb++;

}
MidiEditor::~MidiEditor()
{
    _mdb--;
}

MidiTrackPtr MidiEditor::getTrack()
{
    //hard code to track 0 for now
    return song->getTrack(0);
}

void MidiEditor::selectNextNote()
{
    assert(song);
    assert(selection);

    MidiTrackPtr track = getTrack();
    assert(track);
    if (selection->empty()) {
        

        // track should always have something in it, even if it's an end event
        MidiEventPtr first = track->begin()->second;
        assert(first->type == MidiEvent::Type::Note);

        selection->select(first);
    } else {
        assert(selection->size() == 1);
        MidiEventPtr evt = *selection->begin();
        assert(evt->type == MidiEvent::Type::Note);

        // find the event in the track
        auto it = track->findEvent(*evt);
        if (it == track->end()) {
            assert(false);
        }

        // now iterate up from there
        ++it;
        evt = it->second;
        assert(evt->type == MidiEvent::Type::Note);

        selection->select(evt);
    }
}