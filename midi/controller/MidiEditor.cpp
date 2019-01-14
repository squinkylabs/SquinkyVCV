
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
        return it;                  // if we are at the end, give up
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

/**
 * returns track.end if can't find a note
 */
static MidiTrack::const_iterator findPrevNoteOrCurrent(
    MidiTrackPtr track,
    MidiTrack::const_iterator it)
{

    for (bool done = false; !done; ) {

        MidiEventPtr evt = it->second;
        switch (evt->type) {
            case  MidiEvent::Type::Note:
                done = true;                    // if we are on a note, then we can accept that
                break;
            case MidiEvent::Type::End:
                if (it == track->begin()) {
                    return track->end();            // Empty track, can't dec end ptr, so return "fail"
                } else {
                    --it;                           // try prev
                }
                break;
            default:
                assert(false);
                if (it == track->begin()) {
                    return track->end();            // Empty track, can't dec end ptr, so return "fail"
                } else {
                    --it;                           // try prev
                }
                
        }

    }
    return it;
}

static void selectNextNoteOrCurrent(
    MidiTrackPtr track,
    MidiTrack::const_iterator it,
    MidiSelectionModelPtr selection)
{
    it = findNextNoteOrCurrent(track, it);
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
}

static void selectPrevNoteOrCurrent(
    MidiTrackPtr track,
    MidiTrack::const_iterator it,
    MidiSelectionModelPtr selection)
{
    it = findPrevNoteOrCurrent(track, it);
    if (it == track->end()) {
        // If we can't find a good one, give up
        selection->clear();
    } else {
        MidiEventPtr evt = it->second;
        if (evt->type == MidiEvent::Type::End) {
            selection->clear();
        } else {
            selection->select(evt);
        }
    }
}

void MidiEditor::selectNextNote()
{
    assert(song);
    assert(selection);

    MidiTrackPtr track = getTrack();
    assert(track);
    if (selection->empty()) {
        selectNextNoteOrCurrent(track, track->begin(), selection);
    } else {
        assert(selection->size() == 1);         // can't handle multi select yet
        MidiEventPtr evt = *selection->begin();
        assert(evt->type == MidiEvent::Type::Note);

        // find the event in the track
        auto it = track->findEvent(*evt);
        if (it == track->end()) {
            assert(false);
        }
        ++it;
        selectNextNoteOrCurrent(track, it, selection);
    }
}

void MidiEditor::selectPrevNote()
{
    assert(song);
    assert(selection);

    MidiTrackPtr track = getTrack();
    assert(track);
    if (selection->empty()) {
        // for prev, let's do same as next - if nothing selected, select first
        selectPrevNoteOrCurrent(track, --track->end(), selection);
    } else {
        // taken from next..
        assert(selection->size() == 1);         // can't handle multi select yet
        MidiEventPtr evt = *selection->begin();
        assert(evt->type == MidiEvent::Type::Note);

        // find the event in the track
        auto it = track->findEvent(*evt);
        if (it == track->begin()) {
            selection->clear();         // if we are at start, can't dec.unselect
            return;
        }
        --it;
        selectPrevNoteOrCurrent(track, it, selection);
    }
}

void MidiEditor::transpose(float amount)
{
    for (auto ev : *selection) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(ev);       // for now selection is all notes
        note->pitchCV += amount;
    }
}