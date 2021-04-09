
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
    //assert(false);  // write more
    SQINFO("----- write more HeadingTrackerTester --------");
}

void HeadingTrackerTester::testInit() {
    SHeadingList hl;
    HeadingTracker t(hl);

    const size_t elements = int(SHeading::Type::NUM_TYPES);
    assertEQ(t.curHeadingsIndex.size(), elements);
    assertEQ(t.nextHeadingsIndex.size(), elements);

    for (int i = 0; i < elements; ++i) {
        assert(t.curHeadingsIndex[i] < 0);
        assert(t.nextHeadingsIndex[i] < 0);
    }
}

void HeadingTrackerTester::testOneRegion() {
    const size_t elements = int(SHeading::Type::NUM_TYPES);

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
    const size_t elements = int(SHeading::Type::NUM_TYPES);

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
    const size_t elements = int(SHeading::Type::NUM_TYPES);

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
    const size_t elements = int(SHeading::Type::NUM_TYPES);

    // region before global - global has no effect
    SHeadingList hl;
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Region, 0));
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Global, 2));

    HeadingTracker t(hl);
    for (int i = 0; i < elements; ++i) {
        switch (i) {
            case SHeading::Type::Region:
                assert(t.curHeadingsIndex[i] == 0);
                assert(t.nextHeadingsIndex[i] < 0);
                break;
            case SHeading::Type::Global:
                assert(t.curHeadingsIndex[i] < 0);
                assert(t.nextHeadingsIndex[i] < 0);
                break;
            default:
                assert(t.curHeadingsIndex[i] < 0);
                assert(t.nextHeadingsIndex[i] < 0);
        }
    }
}

void HeadingTrackerTester::testRegionAndGlobal2() {
    const size_t elements = int(SHeading::Type::NUM_TYPES);

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
    const size_t elements = int(SHeading::Type::NUM_TYPES);

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
    const size_t elements = int(SHeading::Type::NUM_TYPES);

    SHeadingList hl;
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Region, 0));
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Region, 10));
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Region, 100));

    HeadingTracker t(hl);
    t.nextRegion();

    for (int i = 0; i < elements; ++i) {
        if (i == (int) SHeading::Type::Region) {
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

void testHeadingTracker() {
    HeadingTrackerTester::test();
}