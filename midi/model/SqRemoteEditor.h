#pragma once


#include <functional>
#include <memory>

class MidiTrack;
using MidiTrackPtr = std::shared_ptr<MidiTrack>;

class SqRemoteEditor
{
public:
    using EditCallback = std::function<void(MidiTrackPtr)>;
    static void server_register(EditCallback);

    static void client_announceData(MidiTrackPtr);
private:
    static EditCallback callback;
};