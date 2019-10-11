
#include "ISeqSettings.h"
#include "MidiEditorContext.h"
#include "MidiSelectionModel.h"
#include "MidiSong.h"
#include "NoteScreenScale.h"
#include "TimeUtils.h"

extern int _mdb;

MidiEditorContext::MidiEditorContext(MidiSongPtr song, ISeqSettingsPtr stt) : 
    _song(song),
    _settings(stt)
{
    ++_mdb;
}

MidiEditorContext::~MidiEditorContext()
{
    --_mdb;
}

void MidiEditorContext::setScaler(std::shared_ptr<NoteScreenScale> _scaler)
{
    assert(_scaler);
    scaler = _scaler;
    MidiEditorContextPtr ctx =  shared_from_this();
    scaler->setContext(ctx);
}

void MidiEditorContext::setNewSong(MidiSongPtr song) 
{
    assert(song);
    _song = song;
}

void MidiEditorContext::scrollViewportToCursorPitch()
{
    //printf("scroll v cursor pitch %f, lo = %f hi = %f\n", m_cursorPitch, pitchLow(), pitchHi());
    while (m_cursorPitch < pitchLow()) {
        scrollVertically(-1 * PitchUtils::octave);
    }
    while (m_cursorPitch > pitchHigh()) {
        scrollVertically(1 * PitchUtils::octave);
    }
}

void MidiEditorContext::assertCursorInViewport() const
{
    assertGE(m_cursorTime, m_startTime);
    assertLT(m_cursorTime, m_endTime);
    assertGE(m_cursorPitch, m_pitchLow);
    assertLE(m_cursorPitch, m_pitchHigh);
}
 
void MidiEditorContext::assertValid() const
{
    assert(m_endTime > m_startTime);
    assert(m_pitchHigh >= m_pitchLow);

    assertGE(m_cursorTime, 0);
    assertLE(m_cursorPitch, 10);      // just for now
    assertGE(m_cursorPitch, -10);

    assert(_settings);
    assertCursorInViewport();
}

void MidiEditorContext::scrollVertically(float pitchCV)
{
    m_pitchHigh += pitchCV;
    m_pitchLow += pitchCV;
}

MidiSongPtr MidiEditorContext::getSong() const
{
    MidiSongPtr ret =  _song.lock();
    assert(ret); 
    return ret;
}

MidiEditorContext::iterator_pair MidiEditorContext::getEvents(float preMargin) const
{
    const float startTime = std::max(0.f, m_startTime - preMargin);
    return getEvents(startTime, m_endTime, m_pitchLow, m_pitchHigh);
}

MidiEditorContext::iterator_pair MidiEditorContext::getEvents(float timeLow, float timeHigh, float pitchLow, float pitchHigh) const
{
    assert(timeLow <= timeHigh);
    assert(timeLow >= 0);
    assert(pitchHigh >= pitchLow);

    iterator::filter_func lambda = [this, pitchLow, pitchHigh, timeHigh](MidiTrack::const_iterator ii) {
        const MidiEventPtr me = ii->second;
        bool ret = false;
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(me);
        if (note) {
            ret = note->pitchCV >= pitchLow && note->pitchCV <= pitchHigh;
        }
        if (ret) {
            ret = me->startTime < timeHigh;
        }
        return ret;
    };

    const auto song = getSong();
    const auto track = song->getTrack(this->trackNumber);

    // raw will be pair of track::const_iterator
    const auto rawIterators = track->timeRange(timeLow, timeHigh);

    return iterator_pair(iterator(rawIterators.first, rawIterators.second, lambda),
        iterator(rawIterators.second, rawIterators.second, lambda));
}

bool MidiEditorContext::cursorInViewport() const
{
    if (m_cursorTime < m_startTime) {
        return false;
    }
    if (m_cursorTime >= m_endTime) {
        return false;
    }
    if (m_cursorPitch > m_pitchHigh) {
        return false;
    }
    if (m_cursorPitch < m_pitchLow) {
        return false;
    }

    return true;
}

bool MidiEditorContext::cursorInViewportTime() const
{
    if (m_cursorTime < m_startTime) {
        return false;
    }
    if (m_cursorTime >= m_endTime) {
        return false;
    }

    return true;
}

void MidiEditorContext::adjustViewportForCursor()
{
    if (!cursorInViewportTime()) {
        auto x = TimeUtils::time2barsAndRemainder(2, m_cursorTime);
        m_startTime = std::get<0>(x) * TimeUtils::bar2time(2);
        m_endTime = m_startTime + TimeUtils::bar2time(2);

        assert(m_startTime >= 0);
        assert(m_cursorTime >= m_startTime);
        assert(m_cursorTime <= m_endTime);
    }

    // and to the pitch
    scrollViewportToCursorPitch();
}

MidiTrackPtr MidiEditorContext::getTrack()
{
    MidiSongPtr song = getSong();
    assert(song);
    return song->getTrack(trackNumber);
}

void MidiEditorContext::setCursorToNote(MidiNoteEventPtrC note)
{
    m_cursorTime = note->startTime;
    m_cursorPitch = note->pitchCV;
    adjustViewportForCursor();
}

void MidiEditorContext::setCursorToSelection(MidiSelectionModelPtr selection)
{
    // could be wrong for multi-select
    if (!selection->empty()) {
        MidiEventPtr ev = *selection->begin();
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(ev);
        assert(note);
        if (note) {
            setCursorToNote(note);
        }
    }
}