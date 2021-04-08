
#include "HeadingTracker.h"
#include "asserts.h"

class HeadingTrackerTester {
public:
    static void test();
    static void testOneRegion();

private:
    static void testInit();
};

void HeadingTrackerTester::test() {
    testInit();
    testOneRegion();
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

void testHeadingTracker() {
    HeadingTrackerTester::test();
}