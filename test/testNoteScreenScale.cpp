#include "asserts.h"

#include "MidiViewport.h"
#include "NoteScreenScale.h"

// basic test of x coordinates
static void test0()
{
    // viewport holds single quarter note
    MidiViewportPtr vp = std::make_shared<MidiViewport>();
    vp->startTime = 0;
    vp->endTime = 1;
  
    // let's make one quarter note fill the whole screen
    MidiNoteEvent note;
    note.setPitch(3, 0);
    vp->pitchLow = note.pitchCV;
    vp->pitchHi = note.pitchCV;

    NoteScreenScale n(vp, 100, 100);
    float left = n.midiTimeToX(note);
    float right = left + n.midiTimeTodX(1.0f);
    assertEQ(left, 0);
    assertEQ(right, 100);

    auto bounds = n.midiTimeToHBounds(note);
    assertEQ(bounds.first, 0);
    assertEQ(bounds.second, 100);
}

// basic test of y coordinates
static void test1()
{
    // viewport holds single quarter note
    MidiViewportPtr vp = std::make_shared<MidiViewport>();
    vp->startTime = 0;
    vp->endTime = 1;

    // let's make one quarter note fill the whole screen
    MidiNoteEvent note;
    note.setPitch(3, 0);
    vp->pitchLow = note.pitchCV;
    vp->pitchHi = note.pitchCV;

    NoteScreenScale n(vp, 100, 100);
    auto y = n.midiPitchToY(note);
    auto h = n.noteHeight();
    assertClose(y, 0, .001);
    assertClose(h, 100, .001); 
}

// test of offset x coordinates
// viewport = 1 bar, have an eight not on beat 4
static void test2()
{
    // viewport holds one bar of 4/4
    MidiViewportPtr vp = std::make_shared<MidiViewport>();
    vp->startTime = 0;
    vp->endTime = 4;

    // let's make one eight note
    MidiNoteEvent note;
    note.startTime = 3.f;
    note.duration = .5f;
    note.setPitch(3, 0);
    vp->pitchLow = note.pitchCV;
    vp->pitchHi = note.pitchCV;

    NoteScreenScale n(vp, 100, 100);

    auto bounds = n.midiTimeToHBounds(note);
    assertEQ(bounds.first, 75.f);
    assertEQ(bounds.second, 75.f +  (100.0 /8));
}

// basic test of y coordinates
static void test3()
{
    // viewport holds two pitches
    MidiViewportPtr vp = std::make_shared<MidiViewport>();
    vp->startTime = 0;
    vp->endTime = 1;

    MidiNoteEvent note1, note2;
    note1.setPitch(3, 0);
    note2.setPitch(3, 1);
    vp->pitchLow = note1.pitchCV;
    vp->pitchHi = note2.pitchCV;

    NoteScreenScale n(vp, 100, 100);
    auto h = n.noteHeight();
    assertClose(h, 50, .001);

    // hight pitch should be at top
    auto y = n.midiPitchToY(note2);
    assertClose(y, 0, .001);

}


void testNoteScreenScale()
{
    test0();
    test1();
    test2();
    test3();
}