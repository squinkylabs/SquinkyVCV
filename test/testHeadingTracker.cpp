
#include "HeadingTracker.h"
#include "asserts.h"

class HeadingTrackerTester {
public:
    static void test();
    static void testOneRegion();
    static void testTwoRegions();
    static void testThreeRegions();
private:
    static void testInit();
};

void HeadingTrackerTester::test() {
    testInit();
    testOneRegion();
    testTwoRegions();
    testThreeRegions();
    assert(false);      // write more
}

void HeadingTrackerTester::testInit() {
    SHeadingList hl;
    HeadingTracker t(hl);

    const size_t elements = int(SHeading::Type::NUM_TYPES);
    assertEQ(t.curHeadings.size(), elements);
    assertEQ(t.nextHeadings.size(), elements);

    for (int i = 0; i < elements; ++i) {
        assert(t.curHeadings[i] == nullptr);
        assert(t.nextHeadings[i] == nullptr);
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
            assert(t.curHeadings[i] == nullptr);
            assert(t.nextHeadings[i] == nullptr);
        } else {
            assert(t.curHeadings[i]);
            assert(t.nextHeadings[i] == nullptr);
        }
    }
}

void HeadingTrackerTester::testTwoRegions() {
    const size_t elements = int(SHeading::Type::NUM_TYPES);

    SHeadingList hl;
    SHeadingPtr reg1 = std::make_shared<SHeading>(SHeading::Type::Region, 0);
    hl.push_back(reg1);
    SHeadingPtr reg2 = std::make_shared<SHeading>(SHeading::Type::Region, 2);
    hl.push_back(reg2);

    HeadingTracker t(hl);
    for (int i = 0; i < elements; ++i) {
        if (i != (int)SHeading::Type::Region) {
            assert(t.curHeadings[i] == nullptr);
            assert(t.nextHeadings[i] == nullptr);
        } else {
            assert(t.curHeadings[i] == reg1.get());
            assert(t.nextHeadings[i] == reg2.get());
        }
    }
}

void HeadingTrackerTester::testThreeRegions() {
    const size_t elements = int(SHeading::Type::NUM_TYPES);

    SHeadingList hl;
    SHeadingPtr reg1 = std::make_shared<SHeading>(SHeading::Type::Region, 0);
    hl.push_back(reg1);
    SHeadingPtr reg2 = std::make_shared<SHeading>(SHeading::Type::Region, 2);
    hl.push_back(reg2);
    SHeadingPtr reg3 = std::make_shared<SHeading>(SHeading::Type::Region, 200);
    hl.push_back(reg3);

    HeadingTracker t(hl);
    for (int i = 0; i < elements; ++i) {
        if (i != (int)SHeading::Type::Region) {
            assert(t.curHeadings[i] == nullptr);
            assert(t.nextHeadings[i] == nullptr);
        }
        else {
            assert(t.curHeadings[i] == reg1.get());
            assert(t.nextHeadings[i] == reg2.get());
        }
    }
}
void testHeadingTracker() {
    HeadingTrackerTester::test();
}