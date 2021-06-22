
#include "GateDelay.h"
#include "asserts.h"

// trivial test - push zeros around
static void test0() {
    GateDelay<2> del;
    del.addGates(SimdBlocks::maskFalse());
    float_4 x = del.getGates();
    simd_assertEQ(x, float_4::zero());
    del.commit();

    del.addGates(SimdBlocks::maskFalse());
    x = del.getGates();
    simd_assertEQ(x, float_4::zero());
    del.commit();

    del.addGates(SimdBlocks::maskFalse());
    x = del.getGates();
    simd_assertEQ(x, float_4::zero());
    del.commit();
}

static void test1(float_4 data) {
    GateDelay<2> del;
    float_4 x;
    int i;
    for (i = 0; i < 4; ++i) {
        del.addGates(data);
        x = del.getGates();
        simd_assertEQ(x, float_4::zero());
    }
    del.commit();

    for (i = 0; i < 4; ++i) {
        del.addGates(data);
        x = del.getGates();
        simd_assertEQ(x, float_4::zero());
    }
    del.commit();

    for (i = 0; i < 4; ++i) {
        del.addGates(data);
        x = del.getGates();
        simd_assertEQ(x, float_4::zero());
    }
    del.commit();

    //  assertEQ(float_4::mask()[1], float_4::mask()[0]);
    for (i = 0; i < 4; ++i) {
        del.addGates(data);
        x = del.getGates();
        assert(SimdBlocks::areMasksEqual(x, data));
    }
    del.commit();
}

static void test1() {
    float_4 t = SimdBlocks::maskTrue();
    float_4 f = SimdBlocks::maskFalse();
    float_4 x0 = t;
    x0[0] = f[0];
    float_4 x1 = t;
    x1[1] = f[0];
    float_4 x2 = t;
    x2[2] = f[0];
    float_4 x3 = t;
    x3[3] = f[0];

    test1(f);
    test1(t);
    test1(x0);
    test1(x1);
    test1(x2);
    test1(x3);
}

// just some sanity checks for some SIMD utilities
static void testMasksEqual() {
    assert(SimdBlocks::areMasksEqual(0, 0));
    assert(SimdBlocks::areMasksEqual(SimdBlocks::maskTrue(), SimdBlocks::maskTrue()));

    auto t = SimdBlocks::maskTrue();
    auto f = float_4::zero();
    assert(SimdBlocks::isChannelTrue(0, t));
    assert(SimdBlocks::isChannelTrue(1, t));
    assert(SimdBlocks::isChannelTrue(2, t));
    assert(SimdBlocks::isChannelTrue(3, t));

    assert(!SimdBlocks::isChannelTrue(0, f));
    assert(!SimdBlocks::isChannelTrue(1, f));
    assert(!SimdBlocks::isChannelTrue(2, f));
    assert(!SimdBlocks::isChannelTrue(3, f));

    float_4 x = t;
    x[0] = 0;
    assert(!SimdBlocks::areMasksEqual(x, f));
    assert(!SimdBlocks::areMasksEqual(f, x));
    x = t;
    x[1] = 0;
    assert(!SimdBlocks::areMasksEqual(x, f));
    assert(!SimdBlocks::areMasksEqual(f, x));
    x = t;
    x[2] = 0;
    assert(!SimdBlocks::areMasksEqual(x, f));
    assert(!SimdBlocks::areMasksEqual(f, x));
    x = t;
    x[3] = 0;
    assert(!SimdBlocks::areMasksEqual(x, f));
    assert(!SimdBlocks::areMasksEqual(f, x));
}

void testGateDelay() {
    testMasksEqual();
    test0();
    test1();
}