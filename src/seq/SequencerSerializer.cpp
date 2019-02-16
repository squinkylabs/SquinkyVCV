
#include "MidiSequencer.h"
#include "SequencerSerializer.h"
#include "jansson.h"


json_t *SequencerSerializer::toJson(MidiSequencerPtr inSeq)
{
    json_t* seq = json_object();
    json_t* song = toJson(inSeq->song); 

    return seq;
}

json_t *SequencerSerializer::toJson(std::shared_ptr<MidiSong>)
{
    json_t* song = json_object();
    assert(false);
    return song;
}

json_t *SequencerSerializer::toJson(std::shared_ptr<MidiTrack>)
{
    json_t* song = json_object();
    assert(false);
    return song;
}

json_t *SequencerSerializer::toJson(std::shared_ptr<MidiEvent> evt)
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

json_t *SequencerSerializer::toJson(std::shared_ptr<MidiNoteEvent>)
{
    json_t* note = json_object();
    assert(false);
    return note;
}

json_t *SequencerSerializer::toJson(std::shared_ptr<MidiEndEvent>)
{
    json_t* end = json_object();
    assert(false);
    return end;
}


void SequencerSerializer::fromJson(json_t *rootJ)
{
}