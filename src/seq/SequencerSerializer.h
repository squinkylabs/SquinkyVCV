#pragma once

#include <memory>

struct json_t;
class MidiEvent;
class MidiNoteEvent;
class MidiEndEvent;
class MidiSequencer;
class MidiSequencer4;
class MidiSong;
class MidiSong4;
class MidiTrack;
class ISeqSettings;
class SequencerModule;
class Sequencer4Module;
class SubrangeLoop;

class SequencerSerializer
{

public:
    static json_t *toJson(std::shared_ptr<MidiSequencer>);
    static json_t *toJson(std::shared_ptr<MidiSequencer4>);
    static std::shared_ptr<MidiSequencer> fromJson(json_t *data, SequencerModule*);
    static std::shared_ptr<MidiSequencer4> fromJson(json_t *data, Sequencer4Module*);

private:
    static json_t *toJson(std::shared_ptr<MidiSong>);
    static json_t *toJson(std::shared_ptr<MidiSong4>);
    static json_t *toJson(std::shared_ptr<MidiTrack>);
    static json_t *toJson(std::shared_ptr<MidiNoteEvent>);
    static json_t *toJson(std::shared_ptr<MidiEndEvent>);
    static json_t *toJson(std::shared_ptr<MidiEvent>);
    static json_t *toJson(std::shared_ptr<ISeqSettings>);
    static json_t *toJson(const SubrangeLoop& loop);

    static std::shared_ptr<MidiSong> fromJsonSong(json_t *data);
    static std::shared_ptr<MidiSong4> fromJsonSong4(json_t *data);
    static MidiTrackPtr fromJsonTrack(json_t *data, int index, std::shared_ptr<MidiLock>);
    static MidiEventPtr fromJsonEvent(json_t *data);
    static MidiNoteEventPtr fromJsonNoteEvent(json_t *data);
    static MidiEndEventPtr fromJsonEndEvent(json_t *data);
    static std::shared_ptr<SubrangeLoop> fromJsonSubrangeLoop(json_t* data);
    static std::shared_ptr<ISeqSettings> fromJsonSettings(json_t* data, SequencerModule*);

    static const int typeNote = 1;
    static const int typeEnd = 2;

    static std::string trackTagForSong4(int row, int col);
};