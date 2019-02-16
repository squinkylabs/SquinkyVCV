#pragma once

#include <memory>

class json_t;
class MidiEvent;
class MidiNoteEvent;
class MidiEndEvent;
class MidiSequencer;
class MidiSong;
class MidiTrack;

class SequencerSerializer
{

public:
    static json_t *toJson(std::shared_ptr<MidiSequencer>);
    static json_t *toJson(std::shared_ptr<MidiSong>);
    static json_t *toJson(std::shared_ptr<MidiTrack>);
    static json_t *toJson(std::shared_ptr<MidiNoteEvent>);
    static json_t *toJson(std::shared_ptr<MidiEndEvent>);
    static json_t *toJson(std::shared_ptr<MidiEvent>);

    static void fromJson(json_t *rootJ);

};