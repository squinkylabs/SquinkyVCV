/**
 * Cross platform helpers go here
 */

#include "InteropClipboard.h"
#include "MidiLock.h"
#include "MidiSelectionModel.h"
#include "MidiTrack.h"
#include "ReplaceDataCommand.h"


void InteropClipboard::PasteData::assertValid()
{
    if (toAdd.empty()) {
        return;
    }
    if (requiredTrackLength < 0) {
        return;
    }
    MidiEventPtr last = toAdd[toAdd.size() - 1];
    assert(last->startTime <= this->requiredTrackLength);
    MidiNoteEventPtr lastNote = safe_cast<MidiNoteEvent>(last);
    assert(lastNote); // true for now
    if (lastNote) {
        assert(lastNote->startTime + lastNote->duration <= this->requiredTrackLength);
    }
}

InteropClipboard::PasteData InteropClipboard::getPasteData(
    float insertTime,
    MidiTrackPtr clipTrack,
    MidiTrackPtr destTrack,
    MidiSelectionModelPtr sel)
{
    assert(clipTrack);
    assert(destTrack);
    assert(sel);        // used to be optional
    clipTrack->assertValid();
    destTrack->assertValid();
    assert(insertTime >= 0);

    PasteData pasteData;

    // all the selected notes get deleted

    for (auto it : *sel) {
        pasteData.toRemove.push_back(it);
    }


    const float eventOffsetTime = insertTime;
    // copy all the notes on the clipboard into the track, but move to insert time

    float newDuration = 0;
    for (auto it : *clipTrack) {
        MidiEventPtr evt = it.second->clone();
        evt->startTime += eventOffsetTime;
        assert(evt->startTime >= 0);
        if (evt->type != MidiEvent::Type::End) {
            pasteData.toAdd.push_back(evt);
            newDuration = std::max(newDuration, evt->startTime);
        }
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(evt);
        if (note) {
            newDuration = std::max(newDuration, note->duration + note->startTime);
        }
    }
    destTrack->assertValid();

    if (sel->isAllSelected()) {
        // if all selected, use the end of the original clip.
        // But make it long enough to hold everything
        float newTrackLength = clipTrack->getLength();
        if (newDuration > newTrackLength) {
            newTrackLength = ReplaceDataCommand::calculateDurationRequest(destTrack, newDuration);
        }
        
        pasteData.requiredTrackLength = newTrackLength;
    } else {
        // If not select all, the make enough room for it, but pad to even bars.   
        const float newTrackLength = ReplaceDataCommand::calculateDurationRequest(destTrack, newDuration);
        pasteData.requiredTrackLength = newTrackLength;
    }
    pasteData.assertValid();
   
    return pasteData;
}

MidiTrackPtr InteropClipboard::getCopyData(MidiTrackPtr track, bool selectAll)
{
    track->assertValid();

    // TODO: move this all to common code
    float firstTime = 0;
    float lastTime = 0;
    
    if (track->begin() != track->end()) {
        MidiEventPtr firstEvent = track->begin()->second;
        if (firstEvent->type != MidiEvent::Type::End) {
            firstTime = firstEvent->startTime;
        }
    }

    MidiLockPtr lock= std::make_shared<MidiLock>();
    MidiLocker l(lock);
    MidiTrackPtr ret = std::make_shared<MidiTrack>(lock);

    for (auto event : *track) {
        MidiEventPtr clone = event.second->clone();
       // const bool isEnd = clone->type == MidiEvent::Type::End;

        if (!selectAll) {
            clone->startTime -= firstTime;
        }
      
        MidiEndEventPtr end = safe_cast<MidiEndEvent>(clone);
        if (end && !selectAll) {
            end->startTime = lastTime;
        }
        ret->insertEvent(clone);

        lastTime = clone->startTime;
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(clone);
        if (note) {
            lastTime += note->duration;
        }

    }
    ret->assertValid();
    return ret;
}