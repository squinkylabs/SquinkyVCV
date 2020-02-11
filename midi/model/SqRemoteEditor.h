#pragma once


#include <functional>
#include <memory>

class MidiTrack;
using MidiTrackPtr = std::shared_ptr<MidiTrack>;

class SqRemoteEditor
{
public:
    using EditCallback = std::function<void(MidiTrackPtr)>;
    static int  serverRegister(EditCallback);
    static void serverUnregister(int);

    static void clientAnnounceData(MidiTrackPtr);
private:
    static EditCallback callback;
    static int theToken;
};