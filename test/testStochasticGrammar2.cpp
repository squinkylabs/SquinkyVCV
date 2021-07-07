#include "asserts.h"
#include "StochasticNote.h"

static void test0() {
    StochasticNote n(StochasticNote::Durations::half);
    assertEQnp(n.duration, StochasticNote::Durations::half);
    assertEQnp(n.tuple, StochasticNote::Tuples::none);

    StochasticNote n2(StochasticNote::Durations::eighth, StochasticNote::Tuples::triplet);
    assertEQnp(n2.duration, StochasticNote::Durations::eighth);
    assertEQnp(n2.tuple, StochasticNote::Tuples::triplet);
   
}

void testStochasticGrammar2() {
    test0();
 
}