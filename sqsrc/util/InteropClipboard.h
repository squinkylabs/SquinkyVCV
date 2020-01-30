#pragma once

#include <memory>
#include <string>

struct json_t;
class MidiTrack;
class MidiEvent;
class MidiNoteEvent;
class MidiLock;

using MidiTrackPtr = std::shared_ptr<MidiTrack>;
using MidiEventPtr = std::shared_ptr<MidiEvent>;
using MidiNoteEventPtr = std::shared_ptr<MidiNoteEvent>;
using MidiLockPtr = std::shared_ptr<MidiLock>;

class InteropClipboard
{
public:
    static void put(MidiTrackPtr);
    static MidiTrackPtr get();
    static void _clear();
private:
    static std::string trackToJson(MidiTrackPtr);

    // this one parses the json string to json data
    static MidiTrackPtr fromJsonStringToTrack(const std::string&, MidiLockPtr lock);
    static MidiTrackPtr fromJsonToTrack(json_t* json, MidiLockPtr lock);
    //static MidiTrackPtr  fromJsonStringToTrack(const std::string& json);
    static MidiEventPtr fromJsonEvent(json_t* json);
    static MidiNoteEventPtr fromJsonNoteEvent(json_t* json);

    static json_t* toJson(MidiTrackPtr tk);
    static json_t* toJson(MidiEventPtr tk);
};