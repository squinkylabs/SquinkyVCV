#pragma once

#include <vector>
#include <map>
#include <memory>
#include "SqCommand.h"

#include "MidiEvent.h"

class MidiTrack;
using MidiTrackPtr = std::shared_ptr<MidiTrack>;
using MidiTrackConstPtr = std::shared_ptr<const MidiTrack>;

class MidiTrack
{
public:
    int size() const;
    bool isValid() const;

    void insertEvent(MidiEventPtr ev);

    // TODO: does this still make sense with polymorphic types?
    void deleteEvent(const MidiEvent&);

    /**
     * Returns all events as a vector, so that they may be indexed.
     * Obviously this is rather slow (O(n)), so don't use it for editing.
     */
    std::vector<MidiEventPtr> _testGetVector() const;


    //

    using container = std::multimap<uint32_t, MidiEventPtr>;

    using iterator = container::iterator;
    using const_iterator = container::const_iterator;
    using iterator_pair = std::pair<const_iterator, const_iterator>;

    /**
     * Returns pair of iterators for all events  start <= t <= end
     */
    iterator_pair timeRange(MidiEvent::time_t start, MidiEvent::time_t end) const;

    iterator begin()
    {
        return events.begin();
    }
    iterator end()
    {
        return events.end();
    }
    const_iterator begin() const
    {
        return events.begin();
    }
    const_iterator end() const
    {
        return events.end();
    }

    /**
     * factory method to generate test content.
     */
    static MidiTrackPtr makeTest1();


private:
    container events;
};

