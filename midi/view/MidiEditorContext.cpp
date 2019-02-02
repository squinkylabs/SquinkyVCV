
#include "MidiEditorContext.h"
#include "MidiSong.h"
#include "TimeUtils.h"

extern int _mdb;

MidiEditorContext::MidiEditorContext(MidiSongPtr song) : _song(song)
{
    ++_mdb;
}

MidiEditorContext::~MidiEditorContext()
{
    --_mdb;
}

void MidiEditorContext::scrollViewportToCursorPitch()
{
    while (m_cursorPitch < pitchLow) {
        scrollVertically(-1 * PitchUtils::octave);
    }
    while (m_cursorPitch > pitchHi) {
        scrollVertically(1 * PitchUtils::octave);
    }
}

void MidiEditorContext::assertCursorInViewport() const
{
    assertGE(m_cursorTime, startTime);
    assertLT(m_cursorTime, endTime);
    assertGE(m_cursorPitch, pitchLow);
    assertLE(m_cursorPitch, pitchHi);
}
 
void MidiEditorContext::assertValid() const
{
    assert(endTime > startTime);
    assert(pitchHi > pitchLow);
    assert(getSong());

    assertGE(m_cursorTime, 0);
    assertLT(m_cursorPitch, 10);      // just for now
    assertGT(m_cursorPitch, -10);

    assertCursorInViewport();
}

void MidiEditorContext::scrollVertically(float pitchCV)
{
    pitchHi += pitchCV;
    pitchLow += pitchCV;
}

std::shared_ptr<const MidiSong> MidiEditorContext::getSong() const
{
    return _song.lock();
}

MidiEditorContext::iterator_pair MidiEditorContext::getEvents() const
{

    iterator::filter_func lambda = [this](MidiTrack::const_iterator ii) {
        const MidiEventPtr me = ii->second;
        bool ret = false;
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(me);
        if (note) {
            ret = note->pitchCV >= pitchLow && note->pitchCV <= pitchHi;
        }
        if (ret) {
            ret = me->startTime < this->endTime;
        }
        return ret;
    };

    const auto song = getSong();
    const auto track = song->getTrack(this->track);

    // raw will be pair of track::const_iterator
    const auto rawIterators = track->timeRange(this->startTime, this->endTime);

    return iterator_pair(iterator(rawIterators.first, rawIterators.second, lambda),
        iterator(rawIterators.second, rawIterators.second, lambda));
}

bool MidiEditorContext::cursorInViewport() const
{
    if (cursorTime() < startTime) {
        return false;
    }
    if (cursorTime() >= endTime) {
        return false;
    }
    if (cursorPitch() > pitchHi) {
        return false;
    }
    if (cursorPitch() < pitchLow) {
        return false;
    }

    return true;
}

bool MidiEditorContext::cursorInViewportTime() const
{
    if (cursorTime() < startTime) {
        return false;
    }
    if (cursorTime() >= endTime) {
        return false;
    }

    return true;
}

void MidiEditorContext::adjustViewportForCursor()
{
    if (!cursorInViewportTime()) {

        float minimumAdvance = 0;
        if (cursorTime() >= endTime) {
            minimumAdvance = cursorTime() - endTime;
        } else if (cursorTime() < startTime) {
            minimumAdvance = cursorTime() - startTime;
        }

        // figure what fraction of 2 bars this is
        float advanceBars = minimumAdvance / TimeUtils::barToTime(2);
        advanceBars += (minimumAdvance < 0) ? -2 : 2;

        float x = std::round(advanceBars / 2.f);
        float finalAdvanceTime = x * TimeUtils::barToTime(2);

        startTime += finalAdvanceTime;
        endTime = startTime + TimeUtils::barToTime(2);
        assert(startTime >= 0);
    }

    // and to the pitch
    scrollViewportToCursorPitch();
}