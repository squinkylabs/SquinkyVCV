
#include "MidiTrack.h"
#include <assert.h>
#include <algorithm>


#ifdef _DEBUG
int MidiEvent::_count = 0;
#endif

int MidiTrack::size() const
{
    return (int) events.size();
}


void MidiTrack::assertValid() const
{
    MidiEvent::time_t startTime = 0;
    for (const_iterator it = begin(); it != end(); ++it) {
        it->second->assertValid();
        assertGE(it->second->startTime, startTime);
        startTime = it->second->startTime;
    }
    // track should end with End event;
    auto it_back = events.rbegin();
    assert(it_back != events.rend());
    if (it_back != events.rend()) {
        assert(it_back->second->type == MidiEvent::Type::End );
    }
}

void MidiTrack::insertEvent(MidiEventPtr evIn)
{
    events.insert( std::pair<MidiEvent::time_t, MidiEventPtr>(evIn->startTime, evIn));
}


void MidiTrack::deleteEvent(const MidiEvent& evIn)
{
    auto candidateRange = events.equal_range(evIn.startTime);
    for (auto it = candidateRange.first; it != candidateRange.second; it++) {
      
        if (*it->second == evIn) {
            events.erase(it);
            return;
        }
    }
    assert(false);
    //  events.insert(insertPoint, std::pair<MidiEvent::time_t, MidiEvent>(evIn.startTime, evIn));
   // events.erase(insertPoint);      // will never work in the real world
}

std::vector<MidiEventPtr> MidiTrack::_testGetVector() const
{
    std::vector<MidiEventPtr> ret;
    std::for_each(events.begin(), events.end(), [&](std::pair<MidiEvent::time_t, const MidiEventPtr&> event) {
        ret.push_back(event.second);
        });
    assert(ret.size() == events.size());

    return ret;
}

MidiTrack::iterator_pair MidiTrack::timeRange(MidiEvent::time_t start, MidiEvent::time_t end) const
{
    return iterator_pair(events.lower_bound(start), events.upper_bound(end));
}

void MidiTrack::insertEnd(MidiEvent::time_t time)
{
    MidiEndEventPtr end = std::make_shared<MidiEndEvent>();
    end->startTime = time;
    insertEvent(end);
}

MidiTrackPtr MidiTrack::makeTest1()
{
    // TODO: don't add the same element multiple times
    auto track =  std::make_shared<MidiTrack>();
    MidiNoteEventPtr ev = std::make_shared<MidiNoteEvent>();
    ev->startTime = 0;
    ev->pitch = 5.0f;
    track->insertEvent(ev);

    ev = std::make_shared<MidiNoteEvent>();
    ev->startTime = 100;
    ev->pitch = 5.1f;
    track->insertEvent(ev);

    ev = std::make_shared<MidiNoteEvent>();
    ev->startTime = 200;
    ev->pitch = 5.2f;
    track->insertEvent(ev);

    ev = std::make_shared<MidiNoteEvent>();
    ev->startTime = 400;
    ev->pitch = 5.3f;
    track->insertEvent(ev);
    track->insertEnd(500);

    return track;
}