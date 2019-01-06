
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
    int numEnds = 0;
    bool lastIsEnd = false;
    (void) lastIsEnd;
    
    float lastEnd = 0;
    MidiEvent::time_t startTime = 0;
    MidiEvent::time_t totalDur = 0;
    for (const_iterator it = begin(); it != end(); ++it) {
        it->second->assertValid();
        assertGE(it->second->startTime, startTime);
        startTime = it->second->startTime;
        if (it->second->type == MidiEvent::Type::End) {
            numEnds++;
            lastIsEnd = true;
            totalDur = startTime;
        } else {
            lastIsEnd = false;
        }
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(it->second);
        if (note) {
            lastEnd = std::max(lastEnd, startTime + note->duration);
        } else {
            lastEnd = startTime;
        }
    }
    assert(lastIsEnd);
    assertEQ(numEnds, 1);
    assertLE(lastEnd, totalDur);
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
    auto track = std::make_shared<MidiTrack>();
    int semi = 0;
    MidiEvent::time_t time = 0;
    for (int i = 0; i < 8; ++i) {
        MidiNoteEventPtr ev = std::make_shared<MidiNoteEvent>();
        ev->startTime = time;
        ev->setPitch(3, semi);
        ev->duration = .5;
        track->insertEvent(ev);

        ++semi;
        time += 1;
    }

    track->insertEnd(time);
    return track;
}
