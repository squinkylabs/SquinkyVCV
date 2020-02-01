/**
 * Cross platform helpers go here
 */

#include "InteropClipboard.h"
#include "MidiLock.h"
#include "MidiTrack.h"


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