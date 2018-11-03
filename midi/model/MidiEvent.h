#pragma once

#include <stdint.h>

class MidiEvent
{
public:
    typedef int32_t time_t;
    uint8_t pitch;      // TODO: why limit us to MIDI range?
    time_t startTime;

    bool operator == (const MidiEvent&) const;
    bool isValid() const;

#ifdef _DEBUG
    MidiEvent() { ++_count; }
    MidiEvent(const MidiEvent& other)
    {
        ++_count;
        *this = other;
    }
    ~MidiEvent() { --_count; }
    static int _count;      // for debugging - reference count
#endif
};

inline bool MidiEvent::operator == (const MidiEvent& other) const
{
    return pitch == other.pitch &&
        startTime == other.startTime;
}

inline bool MidiEvent::isValid() const
{
    return pitch <= 0x7f &&
        startTime >= 0;
}