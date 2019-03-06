#pragma once

#include <memory>

class SqClipboard
{
public:
    class Track
    {
    public:
        std::shared_ptr<class MidiTrack> track;
        float offset = 0;
    };
    enum class DataType
    {
        None,
        Track
    };

    static std::shared_ptr<Track> getTrackData();
    static void putTrackData(std::shared_ptr<Track>);
    static void clear();

private:
    static std::shared_ptr<Track> trackData;
};