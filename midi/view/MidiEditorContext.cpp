
#include "MidiEditorContext.h"
#include "MidiViewport.h"
#include "MidiSong.h"


extern int _mdb;

MidiEditorContext::MidiEditorContext(MidiSongPtr song) : viewport(std::make_shared<MidiViewport>(song))
{
    ++_mdb;
}

MidiEditorContext::~MidiEditorContext()
{
    --_mdb;
}

void MidiEditorContext::scrollViewportToCursorPitch()
{
    while ( cursorPitch < viewport->pitchLow ) {
        viewport->scrollVertically(-1 * PitchUtils::octave);         
    }
    while(cursorPitch > viewport->pitchHi) {
        viewport->scrollVertically(1 * PitchUtils::octave);
    }
}

void MidiEditorContext::assertCursorInViewport() const
{
    assertGE(cursorTime, viewport->startTime);
    assertLT(cursorTime, viewport->endTime);
    assertGE(cursorPitch, viewport->pitchLow);
    assertLE(cursorPitch, viewport->pitchHi);
}

void MidiEditorContext::assertValid() const
{
    assert(viewport);
    viewport->assertValid();
    assertGE(cursorTime, 0);
    assertLT(cursorPitch, 10);      // just for now
    assertGT(cursorPitch, -10);

    assertCursorInViewport();
}