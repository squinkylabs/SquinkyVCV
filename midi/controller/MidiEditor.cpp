
#include <assert.h>
#include "MidiEditor.h"
#include "MidiEditorContext.h"
#include "MidiSelectionModel.h"
#include "MidiSong.h"
#include "MidiTrack.h"
#include "TimeUtils.h"

extern int _mdb;

MidiEditor::MidiEditor(std::shared_ptr<MidiSong> sng,
    std::shared_ptr<MidiSelectionModel> sel,
    std::shared_ptr<MidiEditorContext> ctx) :
    song(sng), selection(sel), context(ctx)
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
    updateCursor();
    context->adjustViewportForCursor();
}

// Move to edit context?
void MidiEditor::updateCursor()
{
    if (selection->empty()) {
        return;
    }

    MidiNoteEventPtr firstNote;
    // If cursor is already in selection, leave it there.
    for (auto it : *selection) {
        MidiEventPtr ev = it;
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(ev);
        if (note) {
            if (!firstNote) {
                firstNote = note;
            }
            if ((note->startTime == context->cursorTime()) &&
                (note->pitchCV == context->cursorPitch())) {
                return;
            }
        }
    }
    context->setCursorTime(firstNote->startTime);
    context->setCursorPitch(firstNote->pitchCV);
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
    updateCursor();
    context->adjustViewportForCursor();
}

void MidiEditor::changePitch(int semitones)
{
    float deltaCV = PitchUtils::semitone * semitones;
    for (auto ev : *selection) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(ev);       // for now selection is all notes
        note->pitchCV += deltaCV;
    }
    context->setCursorPitch(context->cursorPitch() + deltaCV);
    context->adjustViewportForCursor();
    context->assertCursorInViewport();
}

void MidiEditor::changeStartTime(bool ticks, int amount)
{
    assert(!ticks);         // not implemented yet
    assert(amount != 0);
    float advanceAmount = amount * 1.f / 4.f;       // hard code units to 1/16th notes

    MidiNoteEventPtr lastNote = safe_cast<MidiNoteEvent>(selection->getLast());
    float lastTime = lastNote->startTime + lastNote->duration;
    lastTime += advanceAmount;
    extendTrackToMinDuration(lastTime);

    bool setCursor = false;
    MidiTrackPtr track = song->getTrack(0);

    for (auto ev : *selection) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(ev);       // for now selection is all notes
        track->deleteEvent(*note);
        note->startTime += advanceAmount;
        note->startTime = std::max(0.f, note->startTime);
        track->insertEvent(note);
        if (!setCursor) {
            context->setCursorTime(note->startTime);
            setCursor = true;
        }
    }
    context->adjustViewportForCursor();
    context->assertCursorInViewport();
}

void MidiEditor::changeDuration(bool ticks, int amount)
{
    assert(!ticks);         // not implemented yet
    assert(amount != 0);

    float advanceAmount = amount * 1.f / 4.f;       // hard code units to 1/16th notes
    for (auto ev : *selection) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(ev);       // for now selection is all notes
        note->duration += advanceAmount;

        // arbitrary min limit.
        note->duration = std::max(.001f, note->duration);
    }
}

void MidiEditor::assertCursorInSelection()
{
    bool foundIt = false;
    assert(!selection->empty());
    for (auto it : *selection) {
        if (context->cursorTime() == it->startTime) {
            foundIt = true;
        }
    }
    assert(foundIt);
}

void MidiEditor::advanceCursor(bool ticks, int amount)
{
    assert(!ticks);         // not implemented yet
    assert(amount != 0);

    context->assertCursorInViewport();

    float advanceAmount = amount * 1.f / 4.f;       // hard code units to 1/16th notes
    context->setCursorTime(context->cursorTime() + advanceAmount);
    context->setCursorTime(std::max(0.f, context->cursorTime()));
    updateSelectionForCursor();
    context->adjustViewportForCursor();
    context->assertCursorInViewport();
    song->assertValid();
}

void MidiEditor::extendTrackToMinDuration(float neededLength)
{
    auto track = song->getTrack(0);
    float curLength = track->getLength();

    if (neededLength > curLength) {
        float need = neededLength;
        float needBars = need / 4.f;
        float roundedBars = std::round(needBars + 1.f);
        float duration = roundedBars * 4;
        std::shared_ptr<MidiEndEvent> end = track->getEndEvent();
        track->deleteEvent(*end);
        track->insertEnd(duration);
    }
}

void MidiEditor::insertNote()
{
     // for now, fixed to quarter
    extendTrackToMinDuration(context->cursorTime() + 1.f);

    auto track = song->getTrack(0);
    // for now, assume no note there
    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    note->pitchCV = context->cursorPitch();
    note->startTime = context->cursorTime();
    note->duration = 1.0f;          // for now, fixed to quarter
    track->insertEvent(note);
    updateSelectionForCursor();
    song->assertValid();
}

void MidiEditor::deleteNote()
{
    if (selection->empty()) {
        return;
    }

    auto track = song->getTrack(0);
    for (auto it : *selection) {
        MidiEventPtr ev = it;
        track->deleteEvent(*ev);
    }
    selection->clear();
}

void MidiEditor::updateSelectionForCursor()
{
    selection->clear();
    const int cursorSemi = PitchUtils::cvToSemitone(context->cursorPitch());

    // iterator over all the notes that are in the edit context
    auto start = context->startTime;
    auto end = context->endTime;
    MidiTrack::note_iterator_pair notes = getTrack()->timeRangeNotes(start, end);
    for (auto it = notes.first; it != notes.second; ++it) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(it->second);
        const auto startTime = note->startTime;
        const auto endTime = note->startTime + note->duration;

        if ((PitchUtils::cvToSemitone(note->pitchCV) == cursorSemi) &&
            (startTime <= context->cursorTime()) &&
            (endTime > context->cursorTime())) {
            selection->select(note);
            return;
        }
    }
}

void MidiEditor::changeCursorPitch(int semitones)
{
    float pitch = context->cursorPitch() + (semitones * PitchUtils::semitone);
    pitch = std::max(pitch, -5.f);
    pitch = std::min(pitch, 5.f);
    context->setCursorPitch(pitch);
    context->scrollViewportToCursorPitch();
    updateSelectionForCursor();
}

void MidiEditor::setNoteEditorAttribute(MidiEditorContext::NoteAttribute attr)
{
    context->noteAttribute = attr;
}