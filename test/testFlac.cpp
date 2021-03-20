
#include "FlacReader.h"
#include "asserts.h"

static void test0() {
    FlacReader r;
    r.read(nullptr);
}

static void test1() {
    FlacReader r;
    r.read("D:\\samples\\test\\flac\\mono16.flac");
    assert(r.ok());
}

void testFlac()
{
    test0();
    test1();
}