
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

/**
 * If iterator already points to a note, return it.
 * Otherwise search for next one
 */
static MidiTrack::const_iterator findNextNoteOrCurrent(
    MidiTrackPtr track,
    MidiTrack::const_iterator it)
{
    if (it == track->end()) {
        return it;
    }
    for (bool done = false; !done; ) {
       
        if (it == track->end()) {
            done = true;
        }
        MidiEventPtr evt = it->second;
        if (evt->type == MidiEvent::Type::Note) {
            done = true;
        } else if (evt->type == MidiEvent::Type::End) {
            done = true;
        } else {
            assert(false);
            ++it;
        }
    }
    return it;
}

void MidiEditor::selectNextNote()
{
    assert(song);
    assert(selection);

    MidiTrackPtr track = getTrack();
    assert(track);
    if (selection->empty()) {
        // track should always have something in it, even if it's an end event
        auto it = findNextNoteOrCurrent(track, track->begin());
        if (it == track->end()) {
            selection->clear();
        } else {
            MidiEventPtr evt = it->second;
            if (evt->type == MidiEvent::Type::End) {
                selection->clear();
            } else {
                selection->select(evt);
            }
        }

#if 0
        auto it = track->begin();
        for (bool done = false; !done; ) {
            assert(it != track->end());
            MidiEventPtr evt = it->second;
            if (evt->type == MidiEvent::Type::Note) {
                selection->select(evt);
                done = true;
            } else if (evt->type == MidiEvent::Type::End) {
                done = true;
                selection->clear();
            } else {
                assert(false);
                ++it;
            }
        }
#endif
    } else {
        assert(selection->size() == 1);         // can't handle multi select yet
        MidiEventPtr evt = *selection->begin();
        assert(evt->type == MidiEvent::Type::Note);

        // find the event in the track
        auto it = track->findEvent(*evt);
        if (it == track->end()) {
            assert(false);
        }

        // now iterate up from there
        it = findNextNoteOrCurrent(track, ++it);
        if (it == track->end()) {
            selection->clear();
        } else {
            MidiEventPtr evt = it->second;
            if (evt->type == MidiEvent::Type::End) {
                selection->clear();
            } else {
                selection->select(evt);
            }
        }
        /*
        ++it;
        evt = it->second;
        assert(evt->type == MidiEvent::Type::Note);
        

        selection->select(evt);
        */
    }
}