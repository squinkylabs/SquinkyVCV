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
    entry->rhsProducedNotes.push_back(StochasticNote::Durations::eighth);
    entry->rhsProducedNotes.push_back(StochasticNote::Durations::eighth);
    entry->probabilty = .1;

    StochasticProductionRule rule(StochasticNote::Durations::quarter);
    rule.addEntry(entry);
}


class TestEvaluator : public StochasticProductionRule::EvaluationState
{
public:
    TestEvaluator(AudioMath::RandomUniformFunc xr) : StochasticProductionRule::EvaluationState(xr)
    {
    }

    void writeSymbol(int fakeKey) override
    {
      //  keys.push_back(key);
    }

    int getNumSymbols()
    {
        //printf("final keys: ");
       // for (size_t i = 0; i< keys.size(); ++i) printf("%s, ", ProductionRuleKeys::toString(keys[i]));
       // printf("\n");
      //  return (int)keys.size();
        return 0;
    }
private:
   // std::vector<GKEY> keys;
};


//static void testGrammarSub(INITFN f)
static void testGrammarSub()
{
#if 1
   // GKEY init = f();
        std::vector<StochasticProductionRulePtr> grammar;


            // TODO: can we make a verion of is grammar valid? I think so
  //  bool b = ProductionRule::isGrammarValid(rules, numRules, init);
  //  assert(b);

    TestEvaluator es(AudioMath::random());
    es.rules = &grammar;

    StochasticProductionRule::evaluate(es, 0);      // assume first rule is root

    assert(es.getNumSymbols() > 0);
#endif
}


void testStochasticGrammar2() {
    test0();
    testNoteDurations();
    test1();
    testGrammarSub();
}