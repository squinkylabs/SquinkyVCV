
#include "GateDelay.h"
#include "asserts.h"

static void test0() {
    GateDelay<4> del;
    del.addGates( SimdBlocks::maskFalse());
}

void testGateDelay() {
    test0();
}