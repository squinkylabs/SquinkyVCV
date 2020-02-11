#include "SqRemoteEditor.h"

SqRemoteEditor::EditCallback SqRemoteEditor::callback = nullptr;

void SqRemoteEditor::server_register(EditCallback cb)
{
    if (callback) {
        // WARN("editor already registered");
        return;
    }

    callback = cb;
}

void SqRemoteEditor::client_announceData(MidiTrackPtr track)
{
    if (callback) {
        callback(track);
    }
}