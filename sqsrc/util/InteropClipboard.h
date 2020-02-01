#pragma once

#include <memory>
#include <vector>
#include <string>

struct json_t;
class MidiTrack;
class MidiEvent;
class MidiNoteEvent;
class MidiEndEvent;
class MidiSelectionModel;
class MidiLock;

using MidiTrackPtr = std::shared_ptr<MidiTrack>;
using MidiEventPtr = std::shared_ptr<MidiEvent>;
using MidiNoteEventPtr = std::shared_ptr<MidiNoteEvent>;
using MidiEndEventPtr = std::shared_ptr<MidiEndEvent>;
using MidiLockPtr = std::shared_ptr<MidiLock>;
using MidiSelectionModelPtr = std::shared_ptr<MidiSelectionModel>;

class InteropClipboard
{
public:
    class PasteData
    {
    public:
        std::vector<MidiEventPtr> toAdd;
        std::vector<MidiEventPtr> toRemove;
    };
    /**
     * If selectAll is false, trims the track so first note at time 0,
     * length determined by notes.
     * If true, everything absolute.
     * 
     * Note that these high-level funcs have different implementations in unit test.
     */
    static void put(MidiTrackPtr track, bool selectAll);
    static bool empty();

    /**
     * round up the notes to remove and add to implement a
     * paste here.
     */
    static PasteData get(float insertTime, MidiTrackPtr destTrack, MidiSelectionModelPtr sel);

    //static MidiTrackPtr get();
    static void _clear();
    static MidiTrackPtr _getRaw();
private:
    /**
     *  cross-platform implementation functions.
     */
    static MidiTrackPtr getCopyData(MidiTrackPtr track, bool selectAll);


    static PasteData getPasteData(float insertTime, MidiTrackPtr clipTrack, MidiTrackPtr destTrack, MidiSelectionModelPtr sel);


    // first level functions that convert between json strings
    // and MidiTrack
    static std::string trackToJsonString(MidiTrackPtr);
    static MidiTrackPtr fromJsonStringToTrack(const std::string&, MidiLockPtr lock);

    static MidiTrackPtr fromJsonToTrack(MidiLockPtr lock, json_t* notesJson, float length);

    //static MidiTrackPtr  fromJsonStringToTrack(const std::string& json);
    static MidiEventPtr fromJsonEvent(json_t* json);
    static MidiNoteEventPtr fromJsonNoteEvent(json_t* json);

    static json_t* toJson(MidiTrackPtr tk);
    static json_t* toJson(MidiEventPtr tk);
    static json_t* toJson(MidiNoteEventPtr tk);
    static json_t* toJson(MidiEndEventPtr tk);

//cvrack-sequence
    static const char* keyVcvRackSequence;
    static const char* keyLength;
    static const char* keyNotes;
    static const char* keyType;
    static const char* keyNote;
    static const char* keyPitch;
    static const char* keyNoteLength;
    static const char* keyStart;  
};