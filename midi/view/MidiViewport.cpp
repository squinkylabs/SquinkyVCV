#include <assert.h>
#include <iostream>

#include "MidiViewport.h"
#include "MidiSong.h"
#include "FilteredIterator.h"

std::shared_ptr<const MidiSong> MidiViewport::getSong() const
{
    return _song.lock();
}


MidiViewport::iterator_pair MidiViewport::getEvents() const
{

    iterator::filter_func lambda = [this](MidiTrack::const_iterator ii) {
        const MidiEvent me = ii->second;
        const bool ret = me.pitch >= pitchLow && me.pitch <= pitchHi;
        return ret;
    };

    const auto song = getSong();
    const auto track = song->getTrack(this->track);

    // raw will be pair of track::const_iterator
    const auto rawIterators = track->timeRange(this->startTime, this->endTime);

    return iterator_pair(iterator(rawIterators.first, rawIterators.second, lambda),
        iterator(rawIterators.second, rawIterators.second, lambda));
}