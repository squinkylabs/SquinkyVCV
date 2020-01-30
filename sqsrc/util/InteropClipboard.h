#pragma once

#include <memory>
#include <string>

struct json_t;
class MidiTrack;
class MidiEvent;

using MidiTrackPtr = std::shared_ptr<MidiTrack>;
using MidiEventPtr = std::shared_ptr<MidiEvent>;

class InteropClipboard
{
public:
    static void put(MidiTrackPtr);
    static MidiTrackPtr get();
    static void _clear();
private:
    static std::string trackToJson(MidiTrackPtr);
    static MidiTrackPtr  fromJsonToTrack(const std::string& json);

    json_t* toJson(MidiTrackPtr tk);
    json_t* toJson(MidiEventPtr tk);
};