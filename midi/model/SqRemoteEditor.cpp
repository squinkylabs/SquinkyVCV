#include "SqRemoteEditor.h"

SqRemoteEditor::EditCallback SqRemoteEditor::callback = nullptr;

int SqRemoteEditor::theToken = 0;
int SqRemoteEditor::serverRegister(EditCallback cb)
{
    if (callback) {
        // WARN("editor already registered");
        return 0 ;
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
    if (callback) {
        callback(track);
    }
}