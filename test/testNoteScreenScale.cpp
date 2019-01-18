#include "asserts.h"

#include "MidiViewport.h"
#include "NoteScreenScale.h"

static void test0()
{
    // MidiViewport& vp, float screenWidth, float screenHeight
    MidiViewport vp;
    NoteScreenScale n(vp, 100, 100);
}

void testNoteScreenScale()
{
    test0();
}