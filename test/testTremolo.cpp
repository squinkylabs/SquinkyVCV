
#include "Tremolo.h"
#include "TestComposite.h"

using Trem = Tremolo<TestComposite>;

static void test0()
{
    Trem t;
    t.setSampleRate(44100);
    t.init();
    t.step();    
}

void testTremolo()
{
    test0();
}