#include "asserts.h"
#include "StochasticNote.h"

static void test0() {
    StochasticNote n(StochasticNote::Durations::half);
    StochasticNote n2(StochasticNote::Durations::half, StochasticNote::Tuples::triplet);
    (void)n;
}

void testStochasticGrammar2() {
    test0();
 
}