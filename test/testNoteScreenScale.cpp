#include "asserts.h"

#include "MidiViewport.h"
#include "NoteScreenScale.h"

static void test0()
{
    // viewport holds single quarter note
    MidiViewport vp;
    vp.startTime = 0;
    vp.endTime = 1;
  
    // let's make one quarter note fill the whole screen
    MidiNoteEvent note;
    note.setPitch(3, 0);
    vp.pitchLow = note.pitchCV;
    vp.pitchHi = note.pitchCV;

    NoteScreenScale n(vp, 100, 100);
    float left = n.midiTimeToX(note);
    float right = left + n.midiTimeTodX(1.0f);
    assertEQ(left, 0);
    assertEQ(right, 100);
}

void testNoteScreenScale()
{
    test0();
}