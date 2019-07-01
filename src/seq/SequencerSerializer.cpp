
#include "MidiLock.h"
#include "MidiSequencer.h"
#include "SequencerSerializer.h"
#include "SeqSettings.h"
#include "jansson.h"


json_t *SequencerSerializer::toJson(MidiSequencerPtr inSeq)
{
    json_t* seq = json_object();
    json_object_set_new(seq, "song", toJson(inSeq->song));

    return seq;
}

json_t *SequencerSerializer::toJson(std::shared_ptr<MidiSong> sng)
{
    json_t* song = json_object();

    // TODO: more trakcs
    auto tk = sng->getTrack(0);
    json_object_set_new(song, "tk0", toJson(tk));

    return song;
}

json_t *SequencerSerializer::toJson(std::shared_ptr<MidiTrack> tk)
{
    json_t* track = json_array();

    for (auto ev_iter : *tk) {
        MidiEventPtr ev = ev_iter.second;
        json_array_append_new(track, toJson(ev));
    }
    return track;
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

json_t *SequencerSerializer::toJson(std::shared_ptr<MidiNoteEvent> n)
{
    // We could save a little space by omitting type for notes
    json_t* note = json_object();
    json_object_set_new(note, "t", json_integer(typeNote));
    json_object_set_new(note, "s", json_real(n->startTime));
    json_object_set_new(note, "p", json_real(n->pitchCV));
    json_object_set_new(note, "d", json_real(n->duration));
    return note;
}

json_t *SequencerSerializer::toJson(std::shared_ptr<MidiEndEvent> e)
{
    json_t* end = json_object();
    json_object_set_new(end, "t", json_integer(typeEnd));
    json_object_set_new(end, "s", json_real(e->startTime));
    return end;
}

/********************************************************/

/*
    "data": {
        "song": {
          "tk0": [
            {
              "t": 1,
              "s": 0.0,
              "p": 0.0,
              "d": 1.0
            },
            {
              "t": 1,
              "s": 1.75,
              "p": 0.166666672,
              "d": 1.0
            },
            {
              "t": 2,
              "s": 8.0
            }
          ]
        }
      },
      */

MidiSequencerPtr SequencerSerializer::fromJson(json_t *data, rack::engine::Module* module)
{
    json_t* songJson = json_object_get(data, "song");
    MidiSongPtr song = fromJsonSong(songJson);
    ISeqSettings* ss = new SeqSettings(module);
    std::shared_ptr<ISeqSettings> _settings(ss);
    MidiSequencerPtr seq = MidiSequencer::make(song, _settings);
    return seq;
}

MidiSongPtr SequencerSerializer::fromJsonSong(json_t *data)
{
    MidiSongPtr song = std::make_shared<MidiSong>();
    MidiLockPtr lock = song->lock;
    {
        // We must keep song locked to avoid asserts.
        // Must always have lock when changing a track.
        MidiLocker _(lock);

        if (data) {
            json_t* trackJson = json_object_get(data, "tk0");
            MidiTrackPtr track = fromJsonTrack(trackJson, 0, lock);
            song->addTrack(0, track);
        }
    }
    return song;
}

MidiTrackPtr SequencerSerializer::fromJsonTrack(json_t *data, int index, MidiLockPtr lock)
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

MidiEventPtr SequencerSerializer::fromJsonEvent(json_t *data)
{
    MidiEventPtr event;
    json_t* typeJson = json_object_get(data, "t");
    if (!typeJson) {
        printf("bad event\n");
        return event;
    }
    //double json_number_value(const json_t *json)
    int type = json_integer_value(typeJson);
    switch (type) {
        case typeNote:
            event = fromJsonNoteEvent(data);
            break;
        case typeEnd:
            event = fromJsonEndEvent(data);
            break;
        default:
            printf("event type unrecognixed %d\n", type);
    }
    return event;
}

MidiNoteEventPtr SequencerSerializer::fromJsonNoteEvent(json_t *data)
{
    json_t* pitchJson = json_object_get(data, "p");
    json_t* durationJson = json_object_get(data, "d");
    json_t* startJson = json_object_get(data, "s");
    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    note->pitchCV = json_number_value(pitchJson);
    note->duration = json_number_value(durationJson);
    note->startTime = json_number_value(startJson);
    return note;
}

MidiEndEventPtr SequencerSerializer::fromJsonEndEvent(json_t *data)
{
    json_t* startJson = json_object_get(data, "s");
    MidiEndEventPtr end = std::make_shared<MidiEndEvent>();
    end->startTime = json_number_value(startJson);
    return end;
}