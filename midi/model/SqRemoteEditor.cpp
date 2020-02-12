#include "SqRemoteEditor.h"

SqRemoteEditor::EditCallback SqRemoteEditor::callback = nullptr;

int SqRemoteEditor::theToken = 0;
std::weak_ptr<MidiTrack> SqRemoteEditor::lastTrack;

int SqRemoteEditor::serverRegister(EditCallback cb)
{
    assert(cb);
    if (callback) {
        //WARN("editor already registered");
        return 0 ;
    }

    // if server is late to the party, let it
    // know the track to edit.
    MidiTrackPtr track = lastTrack.lock();
    if (track) {
        cb(track); 
    }

    callback = cb;
    theToken = 100;
    return theToken;
}

void SqRemoteEditor::serverUnregister(int t)
{
    if (t == theToken) {
        theToken = 0;
        callback = nullptr;
    }
}

void SqRemoteEditor::clientAnnounceData(MidiTrackPtr track)
{
    lastTrack = track;
    if (callback) {
        callback(track);
    }
}