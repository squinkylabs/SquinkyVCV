#include "StochasticNote.h"
#include "StochasticProductionRule.h"
#include "asserts.h"

static void test0() {
    StochasticNote n(StochasticNote::Durations::half);
    assertEQnp(n.duration, StochasticNote::Durations::half);
    assertEQnp(n.tuple, StochasticNote::Tuples::none);

    StochasticNote n2(StochasticNote::Durations::eighth, StochasticNote::Tuples::triplet);
    assertEQnp(n2.duration, StochasticNote::Durations::eighth);
    assertEQnp(n2.tuple, StochasticNote::Tuples::triplet);
}

static void testNoteDuration(const StochasticNote::Durations dur, double expectedDur) {
    StochasticNote n(dur);
    assertEQ(n.timeDuration(), expectedDur);
}
static void testNoteDurations() {
    testNoteDuration(StochasticNote::Durations::half, .5);
    testNoteDuration(StochasticNote::Durations::quarter, .25);
    testNoteDuration(StochasticNote::Durations::eighth, .125);
    testNoteDuration(StochasticNote::Durations::sixteenth, .25 * .25);
    testNoteDuration(StochasticNote::Durations::thirtysecond, .5 * .25 * .25);
}

static void test1() {
    StochasticProductionRuleEntryPtr entry = StochasticProductionRuleEntry::make();
    entry->rhsProducedNotes.push_back(StochasticNote::Durations::quarter);
    entry->rhsProducedNotes.push_back(StochasticNote::Durations::quarter);
    entry->probabilty = .1;

    StochasticProductionRule rule(StochasticNote::Durations::half);
    rule.addEntry(entry);
}

void testStochasticGrammar2() {
    test0();
    testNoteDurations();
    test1();
}