#pragma once

#include "MidiTrack.h"
#include "FilteredIterator.h"
#include <memory>

class MidiSong;
class MidiSelectionModel;
class NoteScreenScale;

class MidiEditorContext  : public std::enable_shared_from_this<MidiEditorContext>
{
public:
    MidiEditorContext(std::shared_ptr<MidiSong>);
    ~MidiEditorContext();

    float cursorPitch() const
    {
        return m_cursorPitch;
    }
    void setCursorPitch(float pitch)
    {
        assert(pitch <= 10);
        assert(pitch >= -10);
        m_cursorPitch = pitch;
    }
    float cursorTime() const
    {
        return m_cursorTime;
    }
    void setCursorTime(float time)
    {
        m_cursorTime = time;
    }

    MidiEvent::time_t startTime()
    {
        return m_startTime;
    }
    void setStartTime(MidiEvent::time_t t)
    {
        m_startTime = t;
    }
    void setEndTime(MidiEvent::time_t t)
    {
        m_endTime = t;
    }
    void setTimeRange(MidiEvent::time_t start, MidiEvent::time_t end)
    {
        m_startTime = start;
        m_endTime = end;
        assert(end > start);
    }

    MidiEvent::time_t endTime()
    {
        return m_endTime;
    }
    float pitchHigh()
    {
        return m_pitchHigh;
    }
    float pitchLow()
    {
        return m_pitchLow;
    }
    void setPitchLow(float p)
    {
        m_pitchLow = p;
    }
    void setPitchHi(float p)
    {
        m_pitchHigh = p;
    }
    void setPitchRange(float l, float h)
    {
        assert(h >= l);
        assert(h <= 10);
        m_pitchHigh = h;
        m_pitchLow = l;
    }
    int getTrackNumber()
    {
        return trackNumber;
    }
    void setTrackNumber(int n)
    {
        trackNumber = n;
    }

    MidiTrackPtr getTrack();

    void setScaler(std::shared_ptr<NoteScreenScale> _scaler);

    std::shared_ptr<NoteScreenScale> getScaler()
    {
        return scaler;
    }

    void setCursorToNote(MidiNoteEventPtrC note);
    void setCursorToSelection(std::shared_ptr<MidiSelectionModel> selection);

    // TODO: change to const_iterator
    using iterator = filtered_iterator<MidiEvent, MidiTrack::const_iterator>;
    using iterator_pair = std::pair<iterator, iterator>;
    iterator_pair getEvents() const;
    iterator_pair getEvents(float timeLow, float timeHigh, float pitchLow, float pitchHigh) const;

    std::shared_ptr<MidiSong> getSong() const;

    void scrollVertically(float pitchCV);

    // Which field of note is being edited?
    enum class NoteAttribute
    {
        Pitch,
        Duration,
        StartTime
    };

    NoteAttribute noteAttribute = NoteAttribute::Pitch;

    void assertValid() const;

    bool cursorInViewport() const;
    void assertCursorInViewport() const;
    void scrollViewportToCursorPitch();
    bool cursorInViewportTime() const;
    void adjustViewportForCursor();
private:
    float m_cursorTime = 0;
    float m_cursorPitch = 0;

    // range will include t == start time, but will not
    // include t == endTime
    MidiEvent::time_t m_startTime = 0;
    MidiEvent::time_t m_endTime = 1;

    // pitch is inclusive: Low and Hi will be included
    float m_pitchLow = 0;
    float m_pitchHigh = 0;

    int trackNumber = 0;

    std::shared_ptr<NoteScreenScale> scaler;

    // Use weak ref to break circular dependency
    std::weak_ptr<MidiSong> _song;
};

using MidiEditorContextPtr = std::shared_ptr<MidiEditorContext>;