#include <assert.h>
#include <algorithm>
#include "MidiTrack.h"

#ifdef _DEBUG
int MidiEvent::_count = 0;
#endif

int MidiTrack::size() const
{
    return (int) events.size();
}


bool MidiTrack::isValid() const
{
    int32_t startTime = 0;
    for (const_iterator it = begin(); it != end(); ++it) {
        if (!it->second.isValid()) {
            return false;
        }
        if (it->second.startTime < startTime) {
            return false;
        }
        startTime = it->second.startTime;
    }
    return true;
}

void MidiTrack::insertEvent(const MidiEvent& evIn)
{
    events.insert( std::pair<uint32_t, MidiEvent>(evIn.startTime, evIn));
}


void MidiTrack::deleteEvent(const MidiEvent& evIn)
{
    auto candidateRange = events.equal_range(evIn.startTime);
    for (auto it = candidateRange.first; it != candidateRange.second; it++) {
      
        if (it->second == evIn) {
            events.erase(it);
            return;
        }
    }
    assert(false);
    //  events.insert(insertPoint, std::pair<uint32_t, MidiEvent>(evIn.startTime, evIn));
   // events.erase(insertPoint);      // will never work in the real world
}

std::vector<MidiEvent> MidiTrack::_testGetVector() const
{
    std::vector<MidiEvent> ret;
    std::for_each(events.begin(), events.end(), [&](std::pair<uint32_t, const MidiEvent&> event) {
        ret.push_back(event.second);
        });
    assert(ret.size() == events.size());

    return ret;
}

MidiTrack::iterator_pair MidiTrack::timeRange(MidiEvent::time_t start, MidiEvent::time_t end) const
{
    return iterator_pair(events.lower_bound(start), events.upper_bound(end));
}