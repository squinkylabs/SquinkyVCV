
#include "HeadingTracker.h"
#include "asserts.h"

class HeadingTrackerTester {
public:
    static void test();
    static void testOneRegion();
    static void testTwoRegions();
    static void testThreeRegions();
    static void testRegionAndGlobal();
    static void testNext1();
    static void testNext3();

private:
    static void testInit();
};

void HeadingTrackerTester::test() {
    testInit();
    testOneRegion();
    testTwoRegions();
    testThreeRegions();
    testRegionAndGlobal();
    testNext1();
    testNext3();
    assert(false);  // write more
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

    SHeadingList hl;
    //  SHeadingPtr reg1 = std::make_shared<SHeading>(SHeading::Type::Region, 0);
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Region, 0));
    //  SHeadingPtr reg2 = std::make_shared<SHeading>(SHeading::Type::Global, 2);
    hl.push_back(std::make_shared<SHeading>(SHeading::Type::Global, 2));

    HeadingTracker t(hl);
    for (int i = 0; i < elements; ++i) {
        switch (i) {
            case SHeading::Type::Region:
                assert(t.curHeadingsIndex[i] == 0);
                assert(t.nextHeadingsIndex[i] < 0);
                break;
            case SHeading::Type::Global:
                assert(t.curHeadingsIndex[i] == 1);
                assert(t.nextHeadingsIndex[i] < 0);
                break;
            default:
                assert(t.curHeadingsIndex[i] < 0);
                assert(t.nextHeadingsIndex[i] < 0);
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

void testHeadingTracker() {
    HeadingTrackerTester::test();
}