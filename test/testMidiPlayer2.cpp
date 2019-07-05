
#include "MidiPlayer2.h"
#include "MidiVoice.h"
#include "MidiVoiceAssigner.h"

#include "asserts.h"

static void test0()
{
    MidiPlayer2 mp;
    MidiVoice mv;
    MidiVoiceAssigner va(&mv, 1);
    (void) mp;
}

static void basicTestOfMidiVoice()
{
    MidiVoice mv;
    assert(!mv.isPlaying());
}

static void basicTestOfVoiceAssigner()
{
    MidiVoice vx;
    MidiVoiceAssigner va(&vx, 1);
    auto p = va.getNext(0);
    assert(p);
    assert(p == &vx);
}

void testMidiPlayer2()
{
    test0();
    basicTestOfMidiVoice();
    basicTestOfVoiceAssigner();
}