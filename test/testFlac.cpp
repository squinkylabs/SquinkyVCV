
#include "FlacReader.h"
#include "asserts.h"

static void test0() {
    FlacReader r;
    r.read(nullptr);
}

void testFlac()
{
    test0();
}