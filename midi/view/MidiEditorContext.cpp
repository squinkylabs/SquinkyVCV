
#include "MidiEditorContext.h"
//#include "MidiViewport.h"
#include "MidiSong.h"


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
    while (cursorPitch < pitchLow) {
        scrollVertically(-1 * PitchUtils::octave);
    }
    while (cursorPitch > pitchHi) {
        scrollVertically(1 * PitchUtils::octave);
    }
}

void MidiEditorContext::assertCursorInViewport() const
{
    assertGE(cursorTime, startTime);
    assertLT(cursorTime, endTime);
    assertGE(cursorPitch, pitchLow);
    assertLE(cursorPitch, pitchHi);
}
 
void MidiEditorContext::assertValid() const
{
    assert(endTime > startTime);
    assert(pitchHi > pitchLow);
    assert(getSong());

    assertGE(cursorTime, 0);
    assertLT(cursorPitch, 10);      // just for now
    assertGT(cursorPitch, -10);

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