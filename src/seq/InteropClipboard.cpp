#include "SqMidiEvent.h"
#include "InteropClipboard.h"
#include "MidiLock.h"
#include "MidiTrack.h"
#include <asserts.h>

#include "rack.hpp"

bool InteropClipboard::empty()
{
     const char* jsonString = glfwGetClipboardString(APP->window->win);
     return !bool(jsonString);
}

void InteropClipboard::put(MidiTrackPtr trackOrig, bool selectAll)
{
    auto trackToPut = getCopyData(trackOrig, selectAll);
    std::string json = trackToJsonString(trackToPut);
    glfwSetClipboardString(APP->window->win, json.c_str());
}

InteropClipboard::PasteData InteropClipboard::get(
    float insertTime,
    MidiTrackPtr destTrack, 
    MidiSelectionModelPtr sel)
{
    const char* jsonString = glfwGetClipboardString(APP->window->win);
    if (!jsonString) {
        WARN("get clip when empty");
        return PasteData();
    }

    // TODO: pass this in from somewhere?
    // Where did we used to get the lock?
    MidiLockPtr lock = std::make_shared<MidiLock>();
    MidiTrackPtr clipTrack = fromJsonStringToTrack(jsonString, lock );
    if (!clipTrack) {
        WARN("get exit early for incompatible data");
        return PasteData();
    }

    PasteData pasteData = getPasteData(insertTime, clipTrack, destTrack, sel);
    return pasteData;
}

// top level
std::string InteropClipboard::trackToJsonString(MidiTrackPtr track)
{
    json_t* trackJson = toJson(track);
    json_t* rackSequence = json_object();
    json_t* clipboard = json_object();

    // rack sequence has note list and length
    json_object_set_new(rackSequence, keyNotes, trackJson);
    json_object_set_new(rackSequence, keyLength, json_real(track->getLength()));

    // clipboard just has rack Sequence in it
    json_object_set_new(clipboard, keyVcvRackSequence, rackSequence);

    // Use the same formatting and precision as VCV Rack
    std::string clipboardString = json_dumps(clipboard, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
    return clipboardString;
}

// top level
MidiTrackPtr  InteropClipboard::fromJsonStringToTrack(const std::string& json, MidiLockPtr lock)
{
    // parse the string to json
    // it is a clipboard object, from which we will get track
    json_error_t error;
    json_t* clipJ = json_loads(json.c_str(), 0, &error);
	if (!clipJ) {
		WARN("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
		return nullptr;
	}
	DEFER({
		json_decref(clipJ);
	});

    json_t* clipboardJson = json_object_get(clipJ, keyVcvRackSequence);
    if (!clipboardJson) {
        WARN("no sequence data found at root");
        return nullptr;
    }

    float length = 0;
    json_t* notesJson = json_object_get(clipboardJson, keyNotes);
    json_t* jsonLength = json_object_get(clipboardJson, keyLength);

    if (notesJson && jsonLength && json_is_number(jsonLength)) {
        length = json_number_value(jsonLength);
    } else {
        if (!notesJson) WARN("notes missing from sequence clip");
        if (!jsonLength) WARN("length missing from sequence clip");
        return nullptr;
    }

    MidiTrackPtr track = fromJsonToTrack(lock, notesJson, length);
    return track;
}

MidiTrackPtr InteropClipboard::fromJsonToTrack(MidiLockPtr lock, json_t *notesJson, float length )
{
    // data here is the track array
    MidiLocker l(lock);
    MidiTrackPtr track = std::make_shared<MidiTrack>(lock);
    assert(track);
    assert(notesJson);
  
    if (!(json_is_array(notesJson))) {
        WARN("clipboard: notes is not an array");
        return nullptr;
    }

    size_t eventCount = json_array_size(notesJson);
    float lastNoteEnd = 0;  // we don't really want to use this, but for now will keep  
                            // asserts at bay.
    for (int i = 0; i< int(eventCount); ++i) {
        json_t *eventJson = json_array_get(notesJson, i);
        MidiEventPtr event = fromJsonEvent(eventJson);
        if (event) {
            track->insertEvent(event);
            lastNoteEnd = std::max(lastNoteEnd, event->startTime);
        }
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(event);
        if (note) {
            lastNoteEnd = std::max(lastNoteEnd, note->startTime + note->duration);
        }
    }

    if (0 == track->size()) {
        track->insertEnd(0);            // make a legit blank trac
    } else {
        track->insertEnd(lastNoteEnd);
    }

    if (track->getLength() < length) {
        track->setLength(length);
    }

    track->assertValid();
    return track;
}


MidiEventPtr InteropClipboard::fromJsonEvent(json_t *data)
{
    MidiEventPtr event;
    json_t* typeJson = json_object_get(data, keyType);
    if (!typeJson) {
        WARN("clipboard: event has no type");
        return event;
    }

    std::string type = json_string_value(typeJson);
    if (type == keyNote) {
            event = fromJsonNoteEvent(data);
    } else {
        WARN("clipboard: event type unrecognized %s\n", type.c_str());
    }
    return event;
}

MidiNoteEventPtr InteropClipboard::fromJsonNoteEvent(json_t *data)
{
    json_t* pitchJson = json_object_get(data, keyPitch);
    json_t* durationJson = json_object_get(data, keyNoteLength);
    json_t* startJson = json_object_get(data, keyStart);

    if (!json_is_number(pitchJson)) {
        WARN("clipboard: note.pitch is not a number");
        return nullptr;
    }
    if (!json_is_number(durationJson)) {
        WARN("clipboard: note.length is not a number");
        return nullptr;
    }
    if (!json_is_number(startJson)) {
        WARN("clipboard: note.start is not a number");
        return nullptr;
    }

    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    note->pitchCV = json_number_value(pitchJson);
    note->duration = json_number_value(durationJson);
    note->startTime = json_number_value(startJson);

    if (note->duration < 0) {
        WARN("clipboard: note.length < 0");
        return nullptr;
    }

     if (note->startTime < 0) {
        WARN("clipboard: note.start < 0");
        return nullptr;
    }
    return note;
}

json_t *InteropClipboard::toJson(MidiTrackPtr tk)
{
    json_t* track = json_array();

    for (auto ev_iter : *tk) {
        MidiEventPtr ev = ev_iter.second;
        MidiEndEventPtr end = safe_cast<MidiEndEvent>(ev);
        if (!end) {
            json_array_append_new(track, toJson(ev));
        }
    }
    return track;
}

json_t *InteropClipboard::toJson(MidiEventPtr evt)
{
    assert(evt);
    MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(evt);
    if (note) {
        return toJson(note);
    }

    MidiEndEventPtr end = safe_cast<MidiEndEvent>(evt);
    if (end) {
        assert(false);
        return nullptr;
    }

    assert(false);
    return nullptr;
}

json_t *InteropClipboard::toJson(MidiNoteEventPtr n)
{
    json_t* note = json_object();
    json_object_set_new(note, keyType, json_string(keyNote));
    json_object_set_new(note, keyStart, json_real(n->startTime));
    json_object_set_new(note, keyPitch, json_real(n->pitchCV));
    json_object_set_new(note, keyNoteLength, json_real(n->duration));
    return note;
}

const char* InteropClipboard::keyVcvRackSequence = "vcvrack-sequence";
const char* InteropClipboard::keyLength = "length";
const char* InteropClipboard::keyNotes = "notes";

const char* InteropClipboard::keyType = "type";
const char* InteropClipboard::keyNote = "note";
const char* InteropClipboard::keyPitch = "pitch";
const char* InteropClipboard::keyNoteLength = "length";
const char* InteropClipboard::keyStart = "start";  
