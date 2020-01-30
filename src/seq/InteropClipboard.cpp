#include "SqMidiEvent.h"
#include "InteropClipboard.h"
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
    const char* json = glfwGetClipboardString(APP->window->win);
    MidiTrackPtr ret = fromJsonToTrack(json );
}

std::string InteropClipboard::trackToJson(MidiTrackPtr)
{
    assert(false);
}


MidiTrackPtr  InteropClipboard::fromJsonToTrack(const std::string& json)
{
    assert(false);
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
