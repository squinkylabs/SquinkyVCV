#pragma once

#include <memory>

struct json_t;
class MidiEvent;
class MidiNoteEvent;
class MidiEndEvent;
class MidiSequencer;
class MidiSong;
class MidiTrack;
class ISeqSettings;
class SequencerModule;

class SequencerSerializer
{

public:
    static json_t *toJson(std::shared_ptr<MidiSequencer>);
    static std::shared_ptr<MidiSequencer> fromJson(json_t *data, SequencerModule*);

private:
    static json_t *toJson(std::shared_ptr<MidiSong>);
    static json_t *toJson(std::shared_ptr<MidiTrack>);
    static json_t *toJson(std::shared_ptr<MidiNoteEvent>);
    static json_t *toJson(std::shared_ptr<MidiEndEvent>);
    static json_t *toJson(std::shared_ptr<MidiEvent>);
    static json_t *toJson(std::shared_ptr<ISeqSettings>);
    static json_t *toJson(const SubrangeLoop& loop);

    static MidiSongPtr fromJsonSong(json_t *data);
    static MidiTrackPtr fromJsonTrack(json_t *data, int index, std::shared_ptr<MidiLock>);
    static MidiEventPtr fromJsonEvent(json_t *data);
    static MidiNoteEventPtr fromJsonNoteEvent(json_t *data);
    static MidiEndEventPtr fromJsonEndEvent(json_t *data);
    static std::shared_ptr<SubrangeLoop> fromJsonSubrangeLoop(json_t* data);
    static std::shared_ptr<ISeqSettings> fromJsonSettings(json_t* data, SequencerModule*);

    static const int typeNote = 1;
    static const int typeEnd = 2;
};