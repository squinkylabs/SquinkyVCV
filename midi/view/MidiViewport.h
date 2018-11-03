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

    // change to const_iterator
    using iterator = filtered_iterator<MidiEvent, MidiTrack::const_iterator>;
    using iterator_pair = std::pair<iterator, iterator>;
    iterator_pair getEvents() const;

    // This is all pretty note specific
    int32_t startTime = 0;
    int32_t endTime = 0;
    int pitchLow = 0;
    int pitchHi = 0;
    int track = 0;
    std::shared_ptr<const MidiSong> getSong() const;

    // Below is not for clients to call. TODO: use private or something.
    // Definitely need some architecture here.
    std::weak_ptr<MidiSong> _song;
};
