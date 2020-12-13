
#include "WaveLoader.h"
#include "asserts.h"

static void testWaveLoader0()
{
    WaveLoader w;
    w.load("fake file name");
    auto x = w.getInfo(0);
    printf("foo\n");
    assert(!x.valid);
}

static void testWaveLoader1()
{
    WaveLoader w;
    w.load("D:\\samples\\UprightPianoKW-small-SFZ-20190703\\samples\\A3vH.wav");
    auto x = w.getInfo(0);
    printf("foo\n");
    assert(x.valid);
}

void testx2()
{
    testWaveLoader0();
    testWaveLoader1();
    assert(false);
}