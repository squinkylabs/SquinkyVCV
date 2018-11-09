#pragma once

#include <stdint.h>
#include <memory>
#include <assert.h>
#include "asserts.h"

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

    virtual void assertValid() const;

    virtual ~MidiEvent() {
#ifdef _DEBUG
        --_count;
#endif
    }
#ifdef _DEBUG
    static int _count;      // for debugging - reference count
#endif

protected:
    MidiEvent()
    {
#ifdef _DEBUG
        ++_count;
#endif
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

inline void MidiEvent::assertValid() const
{
    assertGE(startTime, 0);
}

/**
 * Derived pointers must provide an implementation for casting
 * base (MidiEventPtr) to derived pointer
 */
template<typename T, typename Q>
inline std::shared_ptr<T> safe_cast(std::shared_ptr<Q>)
{
    // default implementation always fails.
    // this avoids linker errors for unimplemented cases
    return nullptr;
}

using MidiEventPtr = std::shared_ptr<MidiEvent>;



/********************************************************************
**           MidiNoteEvent
********************************************************************/

class MidiNoteEvent : public MidiEvent
{
public:
    MidiNoteEvent()
    {
        type = Type::Note;
    }
    uint8_t pitch=0;      // TODO: why limit us to MIDI range?
    void assertValid() const override;
protected:
    virtual bool isEqual(const MidiEvent&) const override;
};

inline void MidiNoteEvent::assertValid() const
{
    MidiEvent::assertValid();
    assertLE(pitch, 0x7f);
}

inline  bool MidiNoteEvent::isEqual(const MidiEvent& other) const
{
    const MidiNoteEvent* otherNote = static_cast<const MidiNoteEvent*>(&other);
    return other.isEqualBase(*this) && this->pitch == otherNote->pitch;
}


using MidiNoteEventPtr = std::shared_ptr<MidiNoteEvent>;

template<>
inline std::shared_ptr<MidiNoteEvent> safe_cast(std::shared_ptr<MidiEvent> ev)
{
    std::shared_ptr<MidiNoteEvent> note;
    if (ev->type == MidiEvent::Type::Note) {
        note = std::static_pointer_cast<MidiNoteEvent>(ev);
    }
    return note;
}

template<>
inline std::shared_ptr<MidiEvent> safe_cast(std::shared_ptr<MidiNoteEvent> ev)
{
    return ev;
}

/********************************************************************
**           MidiEndEvent
********************************************************************/

class MidiEndEvent : public MidiEvent
{
public:
    void assertValid() const override;
    MidiEndEvent()
    {
        type = Type::End;
    }
protected:
    virtual bool isEqual(const MidiEvent&) const override;
};

inline void MidiEndEvent::assertValid() const
{
    MidiEvent::assertValid();
}

inline  bool MidiEndEvent::isEqual(const MidiEvent& other) const
{
    //const MidiEndEvent* otherNote = static_cast<const MidiEndEvent*>(&other);
    return other.isEqualBase(*this);
}


using MidiEndEventPtr = std::shared_ptr<MidiEndEvent>;

template<>
inline std::shared_ptr<MidiEndEvent> safe_cast(std::shared_ptr<MidiEvent> ev)
{
    std::shared_ptr<MidiEndEvent> note;
    if (ev->type == MidiEvent::Type::End) {
        note = std::static_pointer_cast<MidiEndEvent>(ev);
    }
    return note;
}

template<>
inline std::shared_ptr<MidiEvent> safe_cast(std::shared_ptr<MidiEndEvent> ev)
{
    return ev;
}

