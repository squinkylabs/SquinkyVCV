#include "MidiLock.h"
#include "MidiTrack.h"
#include "TimeUtils.h"

#include <assert.h>
#include <algorithm>
#include <stdio.h>


#ifdef _DEBUG
int MidiEvent::_count = 0;
#endif

MidiTrack::MidiTrack(std::shared_ptr<MidiLock> l) : lock(l)
{

}

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

        // Check for indexing errors
        assertEQ(it->first, it->second->startTime);
    }
    assert(lastIsEnd);
    assertEQ(numEnds, 1);
    assertLE(lastEnd, totalDur);
}

void MidiTrack::insertEvent(MidiEventPtr evIn)
{
    assert(lock);
    assert(lock->locked());
    events.insert(std::pair<MidiEvent::time_t, MidiEventPtr>(evIn->startTime, evIn));
}

float MidiTrack::getLength() const
{
    const_reverse_iterator it = events.rbegin();
    MidiEventPtr end = it->second;
    MidiEndEventPtr ret = safe_cast<MidiEndEvent>(end);
    return ret->startTime;
}
std::shared_ptr<MidiEndEvent> MidiTrack::getEndEvent()
{
    const_reverse_iterator it = events.rbegin();
    MidiEventPtr end = it->second;
    MidiEndEventPtr ret = safe_cast<MidiEndEvent>(end);
    return ret;
}

void MidiTrack::deleteEvent(const MidiEvent& evIn)
{
    assert(lock);
    assert(lock->locked());
    auto candidateRange = events.equal_range(evIn.startTime);
    for (auto it = candidateRange.first; it != candidateRange.second; it++) {

        if (*it->second == evIn) {
            events.erase(it);
            return;
        }
    }
    printf("could not delete event %p\n", &evIn);
    this->_dump();
    fflush(stdout);
    assert(false);          // If you get here it means the event to be deleted was not in the track
}

void MidiTrack::setLength(float newTrackLength)
{
    assert(lock);
    assert(lock->locked());

    MidiEndEventPtr end = getEndEvent();
    assert(end);
    deleteEvent(*end);
    insertEnd(newTrackLength);
    assertValid();
}


void MidiTrack::_dump() const
{
    const_iterator it;
    for (auto it : events) {
        float ti = it.first;
        std::shared_ptr<MidiEvent> evt = it.second;
        std::string type = "Note";
        std::string pitch = "";
        switch (evt->type) {
            case MidiEvent::Type::End:
                type = "End";
                break;
            case MidiEvent::Type::Note:
                type = "Note";
                {
                    MidiNoteEventPtr n = safe_cast<MidiNoteEvent>(evt);
                    char buf[256];
                    snprintf(buf, sizeof(buf), "pitch=%f", n->pitchCV);
                    pitch = buf;
                }
                break;
            case MidiEvent::Type::Test:
            default:
                assert(false);

        }
        const void* addr = evt.get();
        printf("time = %f, type=%s ", ti, type.c_str());
        if (!pitch.empty()) {
            printf(pitch.c_str());
        }
        printf(" addr=%p\n", addr);
    }
    fflush(stdout);
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


MidiTrack::note_iterator_pair MidiTrack::timeRangeNotes(MidiEvent::time_t start, MidiEvent::time_t end) const
{

    note_iterator::filter_func lambda = [this](MidiTrack::const_iterator ii) {
        const MidiEventPtr me = ii->second;
        bool ret = false;
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(me);
        if (note) {
            ret = true;         // accept all notes
        }

        return ret;
    };

    // raw will be pair of track::const_iterator
    const auto rawIterators = this->timeRange(start, end);

    return note_iterator_pair(note_iterator(rawIterators.first, rawIterators.second, lambda),
        note_iterator(rawIterators.second, rawIterators.second, lambda));
}

void MidiTrack::insertEnd(MidiEvent::time_t time)
{
    assert(lock);
    assert(lock->locked());
    MidiEndEventPtr end = std::make_shared<MidiEndEvent>();
    end->startTime = time;
    insertEvent(end);
}

MidiTrack::const_iterator MidiTrack::findEventDeep(const MidiEvent& ev)
{
    iterator_pair range = timeRange(ev.startTime, ev.startTime);
    for (const_iterator it = range.first; it != range.second; ++it) {
        const MidiEventPtr p = it->second;
        if (*p == ev) {
            return it;
        }
    }
    // didn't find it, return end iterator
    return events.end();
}

MidiTrack::const_iterator MidiTrack::seekToTimeNote(MidiEvent::time_t time)
{
    const_iterator it;

    for (it = events.lower_bound(time);
        it != events.end();
        ++it) {

        MidiEventPtr ev = it->second;
        if (ev->type == MidiEvent::Type::Note) {
            return it;
        };
    }
    assert(it == events.end());
    return it;
}

MidiTrack::const_iterator MidiTrack::findEventPointer(MidiEventPtrC ev)
{
    iterator_pair range = timeRange(ev->startTime, ev->startTime);
    for (const_iterator it = range.first; it != range.second; ++it) {
        const MidiEventPtr p = it->second;
        if (p == ev) {
            return it;
        }
    }
    // didn't find it, return end iterator
    return events.end();
}

MidiNoteEventPtr MidiTrack::getFirstNote()
{
    for (auto it : events) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(it.second);
        if (note) {
            return note;
        }
    }
    return nullptr;
}

MidiNoteEventPtr MidiTrack::getSecondNote()
{
    int count = 0;
    for (auto it : events) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(it.second);
        if (note) {
            if (++count == 2) {
                return note;
            }
        }
    }
    return nullptr;
}

MidiNoteEventPtr MidiTrack::getLastNote()
{
    for (auto it = events.rbegin(); it != events.rend(); ++it) {
        MidiEventPtr evt = it->second;
        if (evt->type == MidiEvent::Type::Note) {
            return safe_cast<MidiNoteEvent>(evt);
        }
    }
    return nullptr;
}

MidiTrack::const_iterator MidiTrack::seekToLastNote()
{
    MidiNoteEventPtr note = getLastNote();
    if (!note) {
        return events.end();
    }
    const_iterator it = seekToTimeNote(note->startTime);
    return it;
}

MidiTrackPtr MidiTrack::makeTest(TestContent content, std::shared_ptr<MidiLock> lock)
{
    MidiTrackPtr ret;
    switch (content) {
        case TestContent::eightQNotes:
            ret = makeTest1(lock);
            break;
        case TestContent::empty:
            ret = makeTestEmpty(lock);
            break;
        case TestContent::oneNote123:
            ret = makeTestNote123(lock);
            break;
        case TestContent::oneQ1:
            ret = makeTestOneQ1(lock);
            break;
        case TestContent::FourTouchingQuarters:
            ret = makeTestFourTouchingQuarters(true, lock, false);
            break;
        case TestContent::FourAlmostTouchingQuarters:
            ret = makeTestFourTouchingQuarters(false, lock, false);
            break;
        case TestContent::FourTouchingQuartersOct:
            ret = makeTestFourTouchingQuarters(false, lock, true);
            break;
        default:
            assert(false);
    }
    return ret;
}
/**
 * makes a track of 8 1/4 notes, each of 1/8 note duration (50%).
 * pitch is ascending in semitones from 3:0 (c)
 */

MidiTrackPtr MidiTrack::makeTest1(std::shared_ptr<MidiLock> lock)
{
    auto track = std::make_shared<MidiTrack>(lock);
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

MidiTrackPtr MidiTrack::makeTestNote123(std::shared_ptr<MidiLock> lock)
{
    auto track = std::make_shared<MidiTrack>(lock);
    MidiNoteEventPtr newNote = std::make_shared<MidiNoteEvent>();
    const float testTime = 1.23f;
    newNote->startTime = testTime;
    newNote->duration = 1;
    newNote->pitchCV = 2.3f;

    track->insertEvent(newNote);
    track->insertEnd(TimeUtils::bar2time(1));
    return track;
}

MidiTrackPtr MidiTrack::makeTestOneQ1(std::shared_ptr<MidiLock> lock)
{
    auto track = std::make_shared<MidiTrack>(lock);
    MidiNoteEventPtr newNote = std::make_shared<MidiNoteEvent>();
    const float testTime = 1.;
    newNote->startTime = testTime;
    newNote->duration = 1;
    newNote->pitchCV = 3.0f;

    track->insertEvent(newNote);
    track->insertEnd(TimeUtils::bar2time(1));
    return track;
}

MidiTrackPtr MidiTrack::makeTestFourTouchingQuarters(
    bool exactDuration,
    std::shared_ptr<MidiLock> lock,
    bool spacePitchesByOctave)
{
    auto track = std::make_shared<MidiTrack>(lock);

    const float duration = exactDuration ? 1.f : .999f;
    float pitch = 3.f;
    for (int i = 0; i < 4; ++i) {
        MidiNoteEventPtr newNote = std::make_shared<MidiNoteEvent>();
        newNote->startTime = float(i);
        newNote->duration = duration;
        newNote->pitchCV = pitch;
        if (spacePitchesByOctave) {
            pitch += 1;
        }

        track->insertEvent(newNote);
    }
    track->insertEnd(TimeUtils::bar2time(1));
    return track;
}

MidiTrackPtr MidiTrack::makeTestEmpty(std::shared_ptr<MidiLock> lock)
{
    auto track = std::make_shared<MidiTrack>(lock);
    track->insertEnd(8.f);                  // make two empty bars
    return track;
}

