#pragma once

#include "MidiTrack.h"
#include "FilteredIterator.h"

class MidiSong;
/**
 * Provides a description of the data to be displayed in the view.
 */
class MidiViewport
{
public:
    MidiViewport(std::shared_ptr<MidiSong>);
    ~MidiViewport();
    // TODO: change to const_iterator
    using iterator = filtered_iterator<MidiEvent, MidiTrack::const_iterator>;
    using iterator_pair = std::pair<iterator, iterator>;
    iterator_pair getEvents() const;

    // Properties to filter on
    // This is all pretty note specific

    // range will include t == start time, but will not
    // include t == endTime
    MidiEvent::time_t startTime = 0;
    MidiEvent::time_t endTime = 0;

    // pitch is inclusive: Low and Hi will be included
    float pitchLow = 0;
    float pitchHi = 0;
    int track = 0;
    std::shared_ptr<const MidiSong> getSong() const;

    void assertValid();
    void scrollVertically(float pitchCV);

private:
    // Below is not for clients to call. TODO: use private or something.
    // Definitely need some architecture here.
    std::weak_ptr<MidiSong> _song;
};

using MidiViewportPtr = std::shared_ptr< MidiViewport>;

