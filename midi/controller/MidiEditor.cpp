
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

void MidiEditor::selectNext()
{
    assert(song);
    assert(selection);

    assert(selection->empty());     // can only handle this case for now
    MidiTrackPtr track = getTrack();
    assert(track);

    // track should always have something in it, even if it's an end event
    MidiEventPtr first = track->begin()->second;
    assert(first->type == MidiEvent::Type::Note);

    selection->select(first);
}