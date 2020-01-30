#include "SqMidiEvent.h"
#include "InteropClipboard.h"
#include "MidiLock.h"
#include "MidiTrack.h"
#include <asserts.h>

#include "rack.hpp"


void InteropClipboard::put(MidiTrackPtr track)
{
    std::string json = trackToJson(track);
    glfwSetClipboardString(APP->window->win, json.c_str());
}

MidiTrackPtr InteropClipboard::get()
{
    const char* jsonString = glfwGetClipboardString(APP->window->win);

    // TODO: pass this in from somewhere?
    // Where did we used to get the lock?
    MidiLockPtr lock = std::make_shared<MidiLock>();
    MidiTrackPtr ret = fromJsonStringToTrack(jsonString, lock );
}

std::string InteropClipboard::trackToJson(MidiTrackPtr track)
{
    assert(false);
    return "";
}

#if 1
MidiTrackPtr InteropClipboard::fromJsonToTrack(json_t *data, MidiLockPtr lock)
{
    // data here is the track array
    MidiTrackPtr track = std::make_shared<MidiTrack>(lock);

    size_t eventCount = json_array_size(data);

    for (int i = 0; i< int(eventCount); ++i) {
        json_t *eventJson = json_array_get(data, i);
        MidiEventPtr event = fromJsonEvent(eventJson);
        track->insertEvent(event);
    }
    if (0 == track->size()) {
        printf("bad track\n"); fflush(stdout);
        track->insertEnd(4);            // make a legit blank trac
    }
    return track;
}
#endif

MidiTrackPtr  InteropClipboard::fromJsonStringToTrack(const std::string& json, MidiLockPtr lock)
{
    // parse the string to json
    json_error_t error;
    json_t* clipJ = json_loads(json.c_str(), 0, &error);
	if (!clipJ) {
		WARN("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
		return nullptr;
	}
	DEFER({
		json_decref(clipJ);
	});

    // parse the json to track
   // assert(false);
   MidiTrackPtr track = fromJsonToTrack(clipJ, lock);
   return track;
}

MidiEventPtr InteropClipboard::fromJsonEvent(json_t *data)
{
    MidiEventPtr event;
    json_t* typeJson = json_object_get(data, "t");
    if (!typeJson) {
        printf("bad event\n");
        return event;
    }
    //double json_number_value(const json_t *json)
    std::string type = json_string_value(typeJson);
    // TODO: use constants
    if (type == "note") {
            event = fromJsonNoteEvent(data);
    } else {
        WARN("event type unrecognixed %d\n", type);
    }
    return event;
}

MidiNoteEventPtr InteropClipboard::fromJsonNoteEvent(json_t *data)
{
    json_t* pitchJson = json_object_get(data, "pitch");
    json_t* durationJson = json_object_get(data, "length");
    json_t* startJson = json_object_get(data, "start");
    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    note->pitchCV = json_number_value(pitchJson);
    note->duration = json_number_value(durationJson);
    note->startTime = json_number_value(startJson);
    return note;
}



json_t *InteropClipboard::toJson(MidiTrackPtr tk)
{
    json_t* track = json_array();

    for (auto ev_iter : *tk) {
        MidiEventPtr ev = ev_iter.second;
        json_array_append_new(track, toJson(ev));
    }
    return track;
}

json_t *InteropClipboard::toJson(MidiEventPtr evt)
{
    MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(evt);
    if (note) {
        return toJson(note);
    }

    MidiEndEventPtr end = safe_cast<MidiEndEvent>(evt);
    if (end) {
        return toJson(end);
    }

    assert(false);
    return nullptr;
}


