#include "asserts.h"

#include "MidiEditorContext.h"
#include "NoteScreenScale.h"

// basic test of x coordinates
static void test0()
{
    // viewport holds single quarter note
    MidiEditorContextPtr vp = std::make_shared<MidiEditorContext>(nullptr);
    vp->setStartTime(0);
    vp->setEndTime(1);

    // let's make one quarter note fill the whole screen
    MidiNoteEvent note;
    note.setPitch(3, 0);
    vp->setPitchRange(note.pitchCV, note.pitchCV);

    vp->setCursorPitch(note.pitchCV);

    NoteScreenScale n(100, 100, 0, 0);
    n.setContext(vp);
    float left = n.midiTimeToX(note);
    float right = left + n.midiTimeTodX(1.0f);
    assertEQ(left, 0);
    assertEQ(right, 100);

    float l2 = n.midiTimeToX(note.startTime);
    assertEQ(left, l2);

    auto bounds = n.midiTimeToHBounds(note);
    assertEQ(bounds.first, 0);
    assertEQ(bounds.second, 100);

    // check x -> time
    float t0 = n.xToMidiTime(0);
    assert(t0 == 0);
    t0 = n.xToMidiTime(100);
    assert(t0 == 1);

}

// basic test of y coordinates
static void test1()
{
    // viewport holds single quarter note
    MidiEditorContextPtr vp = std::make_shared<MidiEditorContext>(nullptr);
    vp->setTimeRange(0, 1);

    // let's make one quarter note fill the whole screen
    MidiNoteEvent note;
    note.setPitch(3, 0);
    vp->setPitchRange(note.pitchCV, note.pitchCV);
    vp->setCursorPitch(note.pitchCV);

    NoteScreenScale n(100, 100, 0, 0);
    n.setContext(vp);
    auto y = n.midiPitchToY(note);
    auto h = n.noteHeight();
    assertClose(y, 0, .001);
    assertClose(h, 100, .001);

      // check y -> quantized pitch
    float p = n.yToMidiCVPitch(0);
    assertEQ(p, note.pitchCV);
    p = n.yToMidiCVPitch(100);
    assertEQ(p, note.pitchCV);
}

// test of offset x coordinates
// viewport = 1 bar, have an eight-note on beat 4
static void test2()
{
    // viewport holds one bar of 4/4
    MidiEditorContextPtr vp = std::make_shared<MidiEditorContext>(nullptr);
    vp->setTimeRange(0, 4);

    // let's make one eight note
    MidiNoteEvent note;
    note.startTime = 3.f;
    note.duration = .5f;
    note.setPitch(3, 0);
    vp->setPitchRange(note.pitchCV, note.pitchCV);
    vp->setCursorPitch(note.pitchCV);

    NoteScreenScale n(100, 100, 0, 0);
    n.setContext(vp);

    auto bounds = n.midiTimeToHBounds(note);
    assertEQ(bounds.first, 75.f);
    assertEQ(bounds.second, 75.f + (100.0 / 8));

    float x = n.midiTimeToX(note);
    float x2 = n.midiTimeToX(note.startTime);
    assertEQ(x, x2);
}

// basic test of y coordinates
static void test3()
{
    // viewport holds two pitches
    MidiEditorContextPtr vp = std::make_shared<MidiEditorContext>(nullptr);
    vp->setTimeRange(0, 1);

    MidiNoteEvent note1, note2;
    note1.setPitch(3, 0);
    note2.setPitch(3, 1);
    vp->setPitchRange(note1.pitchCV, note2.pitchCV);
    vp->setCursorPitch(note1.pitchCV);

    NoteScreenScale n(100, 100, 0, 0);
    n.setContext(vp);
    auto h = n.noteHeight();
    assertClose(h, 50, .001);

    // hight pitch should be at top
    auto y = n.midiPitchToY(note2);
    assertClose(y, 0, .001);

    // check y -> quantized pitch
    float p = n.yToMidiCVPitch(0);
    assertEQ(p, note2.pitchCV);
    p = n.yToMidiCVPitch(100);
    assertEQ(p, note1.pitchCV);
}


// basic test of x coordinates
// with margins
static void test4()
{
    // viewport holds single quarter note
    MidiEditorContextPtr vp = std::make_shared<MidiEditorContext>(nullptr);
    vp->setStartTime(0);
    vp->setEndTime(1);

    // let's make one quarter note fill the whole screen
    MidiNoteEvent note;
    note.setPitch(3, 0);
    vp->setPitchRange(note.pitchCV, note.pitchCV);

    vp->setCursorPitch(note.pitchCV);

    NoteScreenScale n(100, 100, 10, 0);            // ten pix left and right
    n.setContext(vp);
    
    float left = n.midiTimeToX(note);
    float right = left + n.midiTimeTodX(1.0f);
    assertEQ(left, 10);
    assertEQ(right, 90);

    float l2 = n.midiTimeToX(note.startTime);
    assertEQ(left, l2);

    auto bounds = n.midiTimeToHBounds(note);
    assertEQ(bounds.first, 10);
    assertEQ(bounds.second, 90);

     // check x -> time
    float t0 = n.xToMidiTime(10);
    assert(t0 == 0);
    t0 = n.xToMidiTime(90);
    assert(t0 == 1);
}


// basic test of y coordinates with margin
static void test5()
{
    // viewport holds two pitches
    MidiEditorContextPtr vp = std::make_shared<MidiEditorContext>(nullptr);
    vp->setTimeRange(0, 1);

    MidiNoteEvent note1, note2;
    note1.setPitch(3, 0);
    note2.setPitch(3, 1);
    vp->setPitchRange(note1.pitchCV, note2.pitchCV);
    vp->setCursorPitch(note1.pitchCV);

    // make 20 pix on top
    NoteScreenScale n(100, 100, 0, 20);
    n.setContext(vp);
    auto h = n.noteHeight();
    assertClose(h, 40, .001);

    // high pitch should be at top
    auto y = n.midiPitchToY(note2);
    assertClose(y, 20, .001);

    // check y -> quantized pitch
    float p = n.yToMidiCVPitch(20);
    assertEQ(p, note2.pitchCV);
    p = n.yToMidiCVPitch(100);
    assertEQ(p, note1.pitchCV);
}


// basic test of y coordinates, with re-calc
static void test6()
{
    // viewport holds single quarter note
    MidiEditorContextPtr vp = std::make_shared<MidiEditorContext>(nullptr);
    vp->setTimeRange(0, 1);

    // let's make one quarter note fill the whole screen
    MidiNoteEvent note;
    note.setPitch(3, 0);
    vp->setPitchRange(note.pitchCV, note.pitchCV);
    vp->setCursorPitch(note.pitchCV);

    NoteScreenScale n(100, 100, 0, 0);
    n.setContext(vp);
    auto y = n.midiPitchToY(note);
    auto h = n.noteHeight();
    assertClose(y, 0, .001);
    assertClose(h, 100, .001);

    // check y -> quantized pitch
    float p = n.yToMidiCVPitch(0);
    assertEQ(p, note.pitchCV);
    p = n.yToMidiCVPitch(100);
    assertEQ(p, note.pitchCV);

    // now go to quarter note an octave higher, should still fill the screen
    // This tests that NoteScreenScale tracks the pitch in the edit context.
    note.setPitch(4, 0);
    vp->setPitchRange(note.pitchCV, note.pitchCV);
    vp->setCursorPitch(note.pitchCV);
    y = n.midiPitchToY(note);
    h = n.noteHeight();
    assertClose(y, 0, .001);
    assertClose(h, 100, .001);

     // check y -> quantized pitch
   // n.reCalculate();        // why does the above stuff work??
    p = n.yToMidiCVPitch(0);
    assertEQ(p, note.pitchCV);
    p = n.yToMidiCVPitch(100);
    assertEQ(p, note.pitchCV);
}

// basic test of x coordinates
static void testTimeRange()
{
    // viewport holds single quarter note
    MidiEditorContextPtr vp = std::make_shared<MidiEditorContext>(nullptr);
    const float start = 1000;
    vp->setStartTime(start);
    vp->setEndTime(start+1);

    // let's make one quarter note fill the whole screen
    MidiNoteEvent note;
    note.setPitch(3, 0);
    note.startTime = start;

    vp->setPitchRange(note.pitchCV, note.pitchCV);

    vp->setCursorPitch(note.pitchCV);
    vp->setCursorTime(start);

    NoteScreenScale n(100, 100, 0, 0);
    n.setContext(vp);
    float left = n.midiTimeToX(note);
    float right = left + n.midiTimeTodX(1.0f);
    assertEQ(left, 0);
    assertEQ(right, 100);

    float l2 = n.midiTimeToX(note.startTime);
    assertEQ(left, l2);

    auto bounds = n.midiTimeToHBounds(note);
    assertEQ(bounds.first, 0);
    assertEQ(bounds.second, 100);

    // check x -> time
    float t0 = n.xToMidiTime(0);
    assertEQ(t0, start);
    t0 = n.xToMidiTime(100);
    assertEQ(t0, start+1);

}

static void testPitchQuantize()
{
    // viewport holds two pitches
    MidiEditorContextPtr vp = std::make_shared<MidiEditorContext>(nullptr);
    vp->setTimeRange(0, 1);

    MidiNoteEvent note1, note2;
    note1.setPitch(3, 0);
    note2.setPitch(3, 1);
    vp->setPitchRange(note1.pitchCV, note2.pitchCV);
    vp->setCursorPitch(note1.pitchCV);

    // make 20 pix on top
    NoteScreenScale n(100, 100, 0, 0);
    n.setContext(vp);
  
    // check y -> quantized pitch
    assertEQ(n.yToMidiCVPitch(0), note2.pitchCV);
    assertEQ(n.yToMidiCVPitch(49), note2.pitchCV);

    assertEQ(n.yToMidiCVPitch(100), note1.pitchCV);
    assertEQ(n.yToMidiCVPitch(51), note1.pitchCV);
}

static void testPointInBounds()
{
    MidiEditorContextPtr vp = std::make_shared<MidiEditorContext>(nullptr);
    vp->setTimeRange(0, 1);

    MidiNoteEvent note1, note2;
    note1.setPitch(3, 0);
    note2.setPitch(3, 1);
    vp->setPitchRange(note1.pitchCV, note2.pitchCV);
    vp->setCursorPitch(note1.pitchCV);

    // make 20 pix on top
    NoteScreenScale n(200, 100, 20, 30);
    n.setContext(vp);

    assert(n.isPointInBounds(50, 50));

    assert(!n.isPointInBounds(19, 50));
    assert(!n.isPointInBounds(200-19, 50));

    assert(!n.isPointInBounds(50, 29));
    assert(!n.isPointInBounds(50, 101));

    assert(!n.isPointInBounds(10000, 100000));
    assert(!n.isPointInBounds(-1, -1));
}

// TODO:
// Need tests where x, y are out of bounds
void testNoteScreenScale()
{
    test0();
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    testTimeRange();
    testPitchQuantize();
    testPointInBounds();
}