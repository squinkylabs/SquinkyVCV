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
        const MidiEventPtr me = ii->second;
        bool ret = false;
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(me);
        if (note) {
            ret = note->pitchCV >= pitchLow && note->pitchCV <= pitchHi;
        }
        if (ret) {
            ret = me->startTime < this->endTime;
        }
        return ret;
    };

    const auto song = getSong();
    const auto track = song->getTrack(this->track);

    // raw will be pair of track::const_iterator
    const auto rawIterators = track->timeRange(this->startTime, this->endTime);

    return iterator_pair(iterator(rawIterators.first, rawIterators.second, lambda),
        iterator(rawIterators.second, rawIterators.second, lambda));
}