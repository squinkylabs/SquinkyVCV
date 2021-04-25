
#include "HeadingTracker.h"
#include "SqLog.h"
#include "asserts.h"

class HeadingTrackerTester {
public:
    static void test();
    static void testOneRegion();
    static void testTwoRegions();
    static void testThreeRegions();
    static void testRegionAndGlobal();
    static void testRegionAndGlobal2();
    static void testNext1();
    static void testNext3();
    static void testRegionsAndGroups1();
    static void testDrum();
    static void testReleaseSamples();
    static void testPiano();

private:
    static void testInit();
};

void HeadingTrackerTester::test() {
    testInit();
    testOneRegion();
    testTwoRegions();
    testThreeRegions();
    testRegionAndGlobal();
    testRegionAndGlobal2();
    testNext1();
    testNext3();
    testRegionsAndGroups1();
    testDrum();
    testReleaseSamples();
    testPiano();
}

void HeadingTrackerTester::testInit() {
    SHeadingList hl;
    HeadingTracker t(hl);

    const size_t elements = int(SHeading::Type::NUM_TYPES_INHERIT);
    assertEQ(t.curHeadingsIndex.size(), elements);
    assertEQ(t.nextHeadingsIndex.size(), elements);

    for (int i = 0; i < elements; ++i) {
        assert(t.curHeadingsIndex[i] < 0);
        assert(t.nextHeadingsIndex[i] < 0);
    }
}

void HeadingTrackerTester::testOneRegion() {
    const size_t elements = int(SHeading::Type::NUM_TYPES_INHERIT);

    SHeadingList hl;
    SHeadingPtr reg = std::make_shared<SHeading>(SHeading::Type::Region, 0);
    hl.push_back(reg);

    HeadingTracker t(hl);
    for (int i = 0; i < elements; ++i) {
        if (i != (int)SHeading::Type::Region) {
            assert(t.curHeadingsIndex[i] < 0);
            assert(t.nextHeadingsIndex[i] < 0);
        } else {
            assert(t.curHeadingsIndex[i] >= 0);
            assert(t.nextHeadingsIndex[i] < 0);
        }
    }
}

void HeadingTrackerTester::testTwoRegions() {
    const size_t elements = int(SHeading::Type::NUM_TYPES_INHERIT);

    SHeadingList hl;
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Region, 0));
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Region, 2));

    HeadingTracker t(hl);
    for (int i = 0; i < elements; ++i) {
        if (i != (int)SHeading::Type::Region) {
            assert(t.curHeadingsIndex[i] < 0);
            assert(t.nextHeadingsIndex[i] < 0);
        } else {
            assert(t.curHeadingsIndex[i] == 0);
            assert(t.nextHeadingsIndex[i] == 1);
        }
    }
}

void HeadingTrackerTester::testThreeRegions() {
    const size_t elements = int(SHeading::Type::NUM_TYPES_INHERIT);

    SHeadingList hl;
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Region, 0));
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Region, 2));
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Region, 200));

    HeadingTracker t(hl);
    for (int i = 0; i < elements; ++i) {
        if (i != (int)SHeading::Type::Region) {
            assert(t.curHeadingsIndex[i] < 0);
            assert(t.nextHeadingsIndex[i] < 0);
        } else {
            assert(t.curHeadingsIndex[i] == 0);
            assert(t.nextHeadingsIndex[i] == 1);
        }
    }
}

void HeadingTrackerTester::testRegionAndGlobal() {
    const size_t elements = int(SHeading::Type::NUM_TYPES_INHERIT);

    // region before global - global has no effect
    SHeadingList hl;
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Region, 0));
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Global, 2));

    HeadingTracker t(hl);
    for (int i = 0; i < elements; ++i) {
        switch (i) {
            case SHeading::Type::Region:
                assert(t.curHeadingsIndex[i] == 0);
                break;
            case SHeading::Type::Global:
                assert(t.curHeadingsIndex[i] < 0);
                break;
            default:
                assert(t.curHeadingsIndex[i] < 0);
        }
    }
}

void HeadingTrackerTester::testRegionAndGlobal2() {
    const size_t elements = int(SHeading::Type::NUM_TYPES_INHERIT);

    // region after global, global has effect
    SHeadingList hl;
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Global, 2));
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Region, 0));

    HeadingTracker t(hl);
    for (int i = 0; i < elements; ++i) {
        switch (i) {
            case SHeading::Type::Region:
                assertEQ(t.curHeadingsIndex[i], 1);
                assertLT(t.nextHeadingsIndex[i], 0);
                break;
            case SHeading::Type::Global:
                assertEQ(t.curHeadingsIndex[i], 0);
                assertLT(t.nextHeadingsIndex[i], 0);
                break;
            default:
                assertLT(t.curHeadingsIndex[i], 0);
                assertLT(t.nextHeadingsIndex[i], 0);
        }
    }
}

void HeadingTrackerTester::testNext1() {
    const size_t elements = int(SHeading::Type::NUM_TYPES_INHERIT);

    SHeadingList hl;
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Region, 0));

    HeadingTracker t(hl);
    t.nextRegion();
    for (int i = 0; i < elements; ++i) {
        assert(t.curHeadingsIndex[i] < 0);
        assert(t.nextHeadingsIndex[i] < 0);
    }
}

void HeadingTrackerTester::testNext3() {
    const size_t elements = int(SHeading::Type::NUM_TYPES_INHERIT);

    SHeadingList hl;
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Region, 0));
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Region, 10));
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Region, 100));

    HeadingTracker t(hl);
    t.nextRegion();

    for (int i = 0; i < elements; ++i) {
        if (i == (int)SHeading::Type::Region) {
            assertEQ(t.curHeadingsIndex[i], 1);
            assertEQ(t.nextHeadingsIndex[i], 2);
        } else {
            assert(t.curHeadingsIndex[i] < 0);
            assert(t.nextHeadingsIndex[i] < 0);
        }
    }
}

void HeadingTrackerTester::testRegionsAndGroups1() {
    SHeadingList hl;
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Region, 0));
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Group, 10));
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Region, 100));

    HeadingTracker t(hl);
    // first region should have no group
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Region], 0);
    assertLT(t.curHeadingsIndex[(int)SHeading::Type::Group], 0);
}

void HeadingTrackerTester::testDrum() {
    auto g1 = std::make_shared<SHeading>(SHeading::Type::Group, 10);
    auto g2 = std::make_shared<SHeading>(SHeading::Type::Group, 10);

    auto r1 = std::make_shared<SHeading>(SHeading::Type::Region, 10);
    auto r2 = std::make_shared<SHeading>(SHeading::Type::Region, 10);
    auto r3 = std::make_shared<SHeading>(SHeading::Type::Region, 10);
    auto r4 = std::make_shared<SHeading>(SHeading::Type::Region, 10);
    auto r5 = std::make_shared<SHeading>(SHeading::Type::Region, 10);
    auto r6 = std::make_shared<SHeading>(SHeading::Type::Region, 10);

    SHeadingList hl;
    hl.push_back(g1);
    hl.push_back(r1);
    hl.push_back(r2);
    hl.push_back(r3);

    hl.push_back(g2);
    hl.push_back(r4);
    hl.push_back(r5);
    hl.push_back(r6);

    HeadingTracker t(hl);

    // first region, r1, g1 in effect
    const size_t elements = int(SHeading::Type::NUM_TYPES_INHERIT);
    for (int i = 0; i < elements; ++i) {
        if (i == (int)SHeading::Type::Region) {
            assertEQ(t.curHeadingsIndex[i], 1);
            assertEQ(t.nextHeadingsIndex[i], 2);
        } else if (i == (int)SHeading::Type::Group) {
            assertEQ(t.curHeadingsIndex[i], 0);
            assertEQ(t.nextHeadingsIndex[i], 4);
        } else {
            assert(t.curHeadingsIndex[i] < 0);
            assert(t.nextHeadingsIndex[i] < 0);
        }
    }

    t.nextRegion();
    // second region, r2, g1 in effect
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Region], 2);
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Group], 0);

    t.nextRegion();
    // third region, r3, g1 in effect
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Region], 3);
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Group], 0);

    t.nextRegion();
    // fourth region, r4, g2 in effect
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Region], 5);
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Group], 4);

    assertEQ(t.nextHeadingsIndex[(int)SHeading::Type::Group], -1);


    t.nextRegion();  //5
    // fifth region, r5, g2 in effect
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Region], 6);
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Group], 4);

    assertEQ(t.nextHeadingsIndex[(int)SHeading::Type::Group], -1);

    t.nextRegion();  //6
                     // sixth region, r6, g2 in effect
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Region], 7);
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Group], 4);

    t.nextRegion();  // done
    assertLT(t.curHeadingsIndex[(int)SHeading::Type::Region], 0);
    assert(!t.getCurrent(SHeading::Type::Region));
}

void HeadingTrackerTester::testReleaseSamples() {
    SHeadingList hl;
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Global, 0));
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Region, 0));
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Group, 0));
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Region, 0));

    HeadingTracker t(hl);
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Global], 0);
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Region], 1);
    assertLT(t.curHeadingsIndex[(int)SHeading::Type::Group], 0);

    t.nextRegion();
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Global], 0);
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Region], 3);
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Group], 2);

    t.nextRegion();  // done
    assertLT(t.curHeadingsIndex[(int)SHeading::Type::Region], 0);
    assert(!t.getCurrent(SHeading::Type::Region));
}

void HeadingTrackerTester::testPiano() {
    SHeadingList hl;
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Global, 0));    // 0
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Group, 0));     // 1
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Region, 0));    // 2
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Region, 0));
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Region, 0));
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Region, 0));    // 5

    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Group, 0));     // 6
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Region, 0));    // 7
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Region, 0));
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Region, 0));
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Region, 0));    // 10

    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Group, 0));     // 11
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Region, 0));    // 12
    
    HeadingTracker t(hl);
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Global], 0);
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Group], 1);
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Region], 2);

    // let's track next to find where the bug is in cur
    assertEQ(t.nextHeadingsIndex[(int)SHeading::Type::Group], 6);

    t.nextRegion();
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Global], 0);
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Group], 1);
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Region], 3);

     // let's track next to find where the bug is in cur
    assertEQ(t.nextHeadingsIndex[(int)SHeading::Type::Group], 6);

    t.nextRegion();
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Global], 0);
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Group], 1);
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Region], 4);

     // let's track next to find where the bug is in cur
    assertEQ(t.nextHeadingsIndex[(int)SHeading::Type::Group], 6);

    t.nextRegion();
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Global], 0);
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Group], 1);
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Region], 5);

     // let's track next to find where the bug is in cur
    assertEQ(t.nextHeadingsIndex[(int)SHeading::Type::Group], 6);

    t.nextRegion();
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Global], 0);
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Group], 6);
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Region], 7);

     // let's track next to find where the bug is in cur
    assertEQ(t.nextHeadingsIndex[(int)SHeading::Type::Group], 11);

    t.nextRegion();
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Global], 0);
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Group], 6);
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Region], 8);

    t.nextRegion();
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Global], 0);
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Group], 6);
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Region], 9);

    t.nextRegion();
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Global], 0);
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Group], 6);
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Region], 10);

    t.nextRegion();
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Global], 0);
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Group], 11);
    assertEQ(t.curHeadingsIndex[(int)SHeading::Type::Region], 12);


    t.nextRegion();
    assert(!t.getCurrent(SHeading::Type::Region));



}

void testHeadingTracker() {
    HeadingTrackerTester::test();
}