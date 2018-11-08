#pragma once

#include <stdint.h>
#include <memory>
#include <assert.h>

/**
 * Abstract base class for all events
 */
class MidiEvent
{
public:
    typedef int32_t time_t;
    enum class Type
    {
        Note,
        End
    };

    Type type;
    time_t startTime;

    bool operator == (const MidiEvent&) const;

    virtual bool isValid() const;

    virtual ~MidiEvent() { --_count; }
    static int _count;      // for debugging - reference count

protected:
    MidiEvent()
    {
        ++_count;
    }

public:
    virtual bool isEqualBase(const MidiEvent& other) const
    {
        return this->startTime == other.startTime;
    }
    virtual bool isEqual(const MidiEvent& other) const = 0;
};


inline bool MidiEvent::operator == (const MidiEvent& other) const
{
    if (other.type != this->type) {
        return false;
    }
    return isEqual(other);
}

inline bool MidiEvent::isValid() const
{
    return startTime >= 0;
}


/********************************************************************/

class MidiNoteEvent : public MidiEvent
{
public:
    uint8_t pitch=0;      // TODO: why limit us to MIDI range?
    bool isValid() const;
protected:
    virtual bool isEqual(const MidiEvent&) const override;
};

inline bool MidiNoteEvent::isValid() const
{
    return MidiEvent::isValid() && (pitch <= 0x7f);
}

inline  bool MidiNoteEvent::isEqual(const MidiEvent& other) const
{
    const MidiNoteEvent* otherNote = static_cast<const MidiNoteEvent*>(&other);
    return other.isEqualBase(*this) && this->pitch == otherNote->pitch;
}

using MidiEventPtr = std::shared_ptr<MidiEvent>;
using MidiNoteEventPtr = std::shared_ptr<MidiNoteEvent>;

template<typename T, typename Q>
std::shared_ptr<T> safe_cast(std::shared_ptr<Q>);

template<>
inline std::shared_ptr<MidiNoteEvent> safe_cast(std::shared_ptr<MidiEvent> ev)
{
    std::shared_ptr<MidiNoteEvent> note;
    if (ev->type == MidiEvent::Type::Note) {
        note = std::static_pointer_cast<MidiNoteEvent>(ev);
    }
    return note;
}

