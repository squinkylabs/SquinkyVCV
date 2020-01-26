
#include "MidiLock.h"
#include "MidiSequencer.h"
#include "MidiSequencer4.h"
#include "MidiTrack4Options.h"
#include "MidiSong4.h"
#include "../SequencerModule.h"
#include "../Sequencer4Module.h"
#include "SequencerSerializer.h"
#include "SeqSettings.h"
#include "jansson.h"

/*
schema for seq++
  root:
  {
      "song": <song>,
      "settings" <settings>
  }

  song:
  {
      "tk0": <track>,
      "loop": <loop>
  }

  for 4X4

  song4:
  {
      "tk0_0": <track>,     // format is row_col
      "tk0_1": <track>,
      "tkx0_0": <track extra>

      "op0_0" <options>
  }

  root4:
  {
      "song4": <song4>,
      "globals4": <globals4>
  }
 */

json_t *SequencerSerializer::toJson(MidiSequencerPtr inSeq)
{
    json_t* seq = json_object();
    json_object_set_new(seq, "song", toJson(inSeq->song));
    json_object_set_new(seq, "settings", toJson(inSeq->context->settings()));

    return seq;
}

json_t *SequencerSerializer::toJson(MidiSequencer4Ptr inSeq)
{
    assert(inSeq);
    json_t* seq = json_object();
    json_object_set_new(seq, "song4", toJson(inSeq->song));
  
    return seq;
}

json_t *SequencerSerializer::toJson(std::shared_ptr<MidiSong> sng)
{
    json_t* song = json_object();

    auto tk = sng->getTrack(0);
    json_object_set_new(song, "tk0", toJson(tk));
    json_object_set_new(song, "loop", toJson(sng->getSubrangeLoop()));

    return song;
}
/*
 song4:
  {
      "tk0_0": <track>,     // format is row_col
      "tk0_1": <track>,
      "tkx0_0": <track extra>
  }
  */
json_t *SequencerSerializer::toJson(std::shared_ptr<MidiSong4> sng)
{
    json_t* song = json_object();
    for (int row=0; row < MidiSong4::numTracks; ++row) {
        for (int col=0; col < MidiSong4::numSectionsPerTrack; ++col) {
            {
                const std::string key = trackTagForSong4(row, col);
                auto tk = sng->getTrack(row, col);
                if (tk) {
                    // only serialize tracks that exist
                    json_object_set_new(song, key.c_str(), toJson(tk));
                }
            }

            {
                const std::string key = optionTagForSong4(row, col);
                auto opt = sng->getOptions(row, col);
                WARN("saving song opt = %d", !!opt);
                if (opt) {

                    // only serialize tracks that exist
                    json_object_set_new(song, key.c_str(), toJson(opt));
                }
            }
        }
    }
    return song;
}

std::string SequencerSerializer::trackTagForSong4(int row, int col)
{
    std::stringstream str;
    str << "tk" << row << "_" << col;
    return str.str(); 
}

std::string SequencerSerializer::optionTagForSong4(int row, int col)
{
    std::stringstream str;
    str << "opt" << row << "_" << col;
    return str.str(); 
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

json_t* SequencerSerializer::toJson(const SubrangeLoop& lp)
{
    json_t* loop = json_object();
    json_object_set_new(loop, "enabled", json_boolean(lp.enabled));
    json_object_set_new(loop, "startTime", json_boolean(lp.startTime));
    json_object_set_new(loop, "endTime", json_boolean(lp.endTime));
    return loop;
}


 json_t* SequencerSerializer::toJson(std::shared_ptr<MidiTrack4Options> options)
 {
    WARN("to json options called\n");
    assert(options);
    json_t* opt = json_object();
    json_object_set_new(opt, "repeat", json_integer(options->repeatCount));
    return opt;
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

json_t* SequencerSerializer::toJson(std::shared_ptr<ISeqSettings> settings)
{
    SeqSettings* rawSettings = dynamic_cast<SeqSettings*>(settings.get());
    json_t* jsonSettings = json_object();

    json_object_set_new(jsonSettings, "snapToGrid", json_boolean(settings->snapToGrid()));
    json_object_set_new(jsonSettings, "snapDurationToGrid", json_boolean(settings->snapDurationToGrid()));

    auto grid = rawSettings->getGridString();
    json_object_set_new(jsonSettings, "grid", json_string(grid.c_str()));

    auto artic = rawSettings->getArticString();
    json_object_set_new(jsonSettings, "articulation", json_string(artic.c_str()));

    json_object_set_new(jsonSettings, "midiFilePath", json_string(rawSettings->midiFilePath.c_str()));

    auto keysig = settings->getKeysig();

    json_object_set_new(jsonSettings,  "keysigRoot", json_integer(keysig.first));
    json_object_set_new(jsonSettings,  "keysigMode", json_integer(int(keysig.second)));

    return jsonSettings;
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

MidiSequencerPtr SequencerSerializer::fromJson(json_t *data, SequencerModule* module)
{
    json_t* songJson = json_object_get(data, "song");
    MidiSongPtr song = fromJsonSong(songJson);

    json_t* settingsJson = json_object_get(data, "settings");
    std::shared_ptr<ISeqSettings> _settings = fromJsonSettings(settingsJson, module);

    MidiSequencerPtr seq = MidiSequencer::make(song, _settings, module->seqComp->getAuditionHost());
    return seq;
}

MidiSequencer4Ptr SequencerSerializer::fromJson(json_t *data, Sequencer4Module* module)
{
    json_t* songJson = json_object_get(data, "song4");
    MidiSong4Ptr song = fromJsonSong4(songJson);
    
    MidiSequencer4Ptr seq = MidiSequencer4::make(song);
    return seq;
}

std::shared_ptr<ISeqSettings> SequencerSerializer::fromJsonSettings(
    json_t* data,
    SequencerModule* module)
{
    SeqSettings* rawSettings = new SeqSettings(module);

    ISeqSettings* ss = rawSettings;
    std::shared_ptr<ISeqSettings> _settings(ss);

    if (data) {
        json_t* gridSetting = json_object_get(data, "grid");
        if (gridSetting) {
            std::string gridS = json_string_value(gridSetting);
            SeqSettings::Grids g = SeqSettings::gridFromString(gridS);
            rawSettings->curGrid = g;
        }

        json_t* articSetting = json_object_get(data, "articulation");
        if (articSetting) {
            std::string articS = json_string_value(articSetting);
            SeqSettings::Artics a = SeqSettings::articFromString(articS);
            rawSettings->curArtic = a;
        }

        json_t* snap = json_object_get(data, "snapToGrid");
        if (snap) {
            bool bSnap = json_boolean_value(snap);
            rawSettings->snapEnabled = bSnap;
        }

        json_t* snapDur = json_object_get(data, "snapDurationToGrid");
        if (snapDur) {
            bool bSnap = json_boolean_value(snapDur);
            rawSettings->snapDurationEnabled = bSnap;
        }

        json_t* midiFilePath = json_object_get(data, "midiFilePath");
        if (midiFilePath) {
            std::string path = json_string_value(midiFilePath);
            rawSettings->midiFilePath = path;
        }

        json_t* keysigRoot = json_object_get(data, "keysigRoot");
        if (keysigRoot) {
            int root = json_integer_value(keysigRoot);
            rawSettings->keysigRoot = root;
        }

        json_t* keysigMode = json_object_get(data, "keysigMode");
        if (keysigMode) {
            int mode = json_integer_value(keysigMode);
            rawSettings->keysigMode = Scale::Scales(mode);
        }
    }
    return _settings;
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

            json_t* loopJson = json_object_get(data, "loop");
            if (loopJson) {
                std::shared_ptr<SubrangeLoop> loopPtr = fromJsonSubrangeLoop(loopJson);
                song->setSubrangeLoop(*loopPtr);
            }
        }
    }
    return song;
}

MidiSong4Ptr SequencerSerializer::fromJsonSong4(json_t *data)
{
    MidiSong4Ptr song = std::make_shared<MidiSong4>();
    MidiLockPtr lock = song->lock;
    {
        // We must keep song locked to avoid asserts.
        // Must always have lock when changing a track.
        MidiLocker _(lock);


        if (data) {
            for (int row=0; row < MidiSong4::numTracks; ++row) {
                for (int col=0; col < MidiSong4::numSectionsPerTrack; ++col) {

                    {
                        std::string key = trackTagForSong4(row, col);
                        json_t* trackJson = json_object_get(data, key.c_str());
                        MidiTrackPtr track;
                        if (trackJson) {
                            track = fromJsonTrack(trackJson, 0, lock);
                        }
                        song->addTrack(row, col, track);
                    }
                    {
                        std::string key = optionTagForSong4(row, col);
                        json_t* optionJson = json_object_get(data, key.c_str());
                        MidiTrack4OptionsPtr options;
                        if (optionJson) {
                            options = fromJsonOptions(optionJson);
                        }
                        song->addOptions(row, col, options);
                    }
                }
            }
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

std::shared_ptr<SubrangeLoop> SequencerSerializer::fromJsonSubrangeLoop(json_t* data)
{
    json_t* enableJson = json_object_get(data, "enabled");
    json_t* startJson = json_object_get(data, "startTime");
    json_t* endJson = json_object_get(data, "endTime");

    std::shared_ptr<SubrangeLoop> loop = std::make_shared<SubrangeLoop>();
    loop->enabled = json_boolean_value(enableJson);
    loop->startTime = json_number_value(startJson);
    loop->endTime = json_number_value(endJson);
    return loop;
}

MidiTrack4OptionsPtr SequencerSerializer::fromJsonOptions(json_t* data )
{
    MidiTrack4OptionsPtr options = std::make_shared<MidiTrack4Options>();
    json_t* repeatJson = json_object_get(data, "repeat");
    options->repeatCount = json_integer_value(repeatJson);
    return options;
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