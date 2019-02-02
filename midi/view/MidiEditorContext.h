#pragma once

#include "MidiTrack.h"
#include "FilteredIterator.h"
#include <memory>

///class MidiViewport;
class MidiSong;

class MidiEditorContext
{
public:
    MidiEditorContext(std::shared_ptr<MidiSong>);
    ~MidiEditorContext();

    // TODO: don't allow direct access?
    float cursorTime = 0;
    float cursorPitch = 0;

       // TODO: change to const_iterator
    using iterator = filtered_iterator<MidiEvent, MidiTrack::const_iterator>;
    using iterator_pair = std::pair<iterator, iterator>;
    iterator_pair getEvents() const;

    // Properties to filter on
    // This is all pretty note specific

    // range will include t == start time, but will not
    // include t == endTime
    MidiEvent::time_t startTime = 0;
    MidiEvent::time_t endTime = 0;

    // pitch is inclusive: Low and Hi will be included
    float pitchLow = 0;
    float pitchHi = 0;
    int track = 0;
    std::shared_ptr<const MidiSong> getSong() const;

  //  void assertValid();
    void scrollVertically(float pitchCV);

    // Which field of note is being edited?
    enum class NoteAttribute
    {
        Pitch,
        Duration,
        StartTime
    };

    NoteAttribute noteAttribute;

    void assertValid() const;

    void assertCursorInViewport() const;
    void scrollViewportToCursorPitch();
private:
     // Below is not for clients to call. TODO: use private or something.
    // Definitely need some architecture here.
    std::weak_ptr<MidiSong> _song;

};

using MidiEditorContextPtr = std::shared_ptr<MidiEditorContext>;