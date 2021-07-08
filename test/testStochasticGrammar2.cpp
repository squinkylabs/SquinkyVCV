#include "StochasticGrammar2.h"
#include "StochasticNote.h"
#include "StochasticProductionRule.h"
#include "asserts.h"

static void test0() {
    StochasticDisplayNote n(StochasticDisplayNote::Durations::half);
    assertEQnp(n.duration, StochasticDisplayNote::Durations::half);
    assertEQnp(n.tuple, StochasticDisplayNote::Tuples::none);

    StochasticDisplayNote n2(StochasticDisplayNote::Durations::eighth, StochasticDisplayNote::Tuples::triplet);
    assertEQnp(n2.duration, StochasticDisplayNote::Durations::eighth);
    assertEQnp(n2.tuple, StochasticDisplayNote::Tuples::triplet);
}

static void testNoteDuration(const StochasticDisplayNote::Durations dur, double expectedDur) {
    StochasticDisplayNote n(dur);
    assertEQ(n.timeDuration(), expectedDur);
}

static void testNoteDurations() {
    testNoteDuration(StochasticDisplayNote::Durations::half, .5);
    testNoteDuration(StochasticDisplayNote::Durations::quarter, .25);
    testNoteDuration(StochasticDisplayNote::Durations::eighth, .125);
    testNoteDuration(StochasticDisplayNote::Durations::sixteenth, .25 * .25);
    testNoteDuration(StochasticDisplayNote::Durations::thirtysecond, .5 * .25 * .25);
}

static void test1() {
    StochasticProductionRuleEntryPtr entry = StochasticProductionRuleEntry::make();
    entry->rhsProducedNotes.push_back(StochasticNote(StochasticDisplayNote::Durations::eighth));
    entry->rhsProducedNotes.push_back(StochasticNote(StochasticDisplayNote::Durations::eighth));
    entry->probabilty = .1;

    StochasticDisplayNote dn = StochasticDisplayNote::Durations::quarter;
    StochasticNote n(dn);
    StochasticProductionRule rule(n);
    rule.addEntry(entry);
}


class TestEvaluator : public StochasticProductionRule::EvaluationState
{
public:
    TestEvaluator(AudioMath::RandomUniformFunc xr) : StochasticProductionRule::EvaluationState(xr)
    {
    }

    void writeSymbol(const StochasticNote& sym) override
    {
       // assert(false);
      //  keys.push_back(key);
        notes.push_back(sym);
    }

    size_t getNumSymbols()
    {
        //printf("final keys: ");
       // for (size_t i = 0; i< keys.size(); ++i) printf("%s, ", ProductionRuleKeys::toString(keys[i]));
       // printf("\n");
      //  return (int)keys.size();
        return notes.size();
    }
private:
    std::vector<StochasticNote> notes;
};



static StochasticGrammar getGrammar() {
 //   assert(false);
 //   return StochasticGrammar();
    StochasticGrammar gmr;


    auto rootRule = std::make_shared<StochasticProductionRule>(StochasticDisplayNote(StochasticDisplayNote::Durations::half));
    auto entry = StochasticProductionRuleEntry::make();
    entry->rhsProducedNotes.push_back(StochasticNote(StochasticDisplayNote::Durations::quarter));
    entry->rhsProducedNotes.push_back(StochasticNote(StochasticDisplayNote::Durations::quarter));
    entry->probabilty = .5;
    rootRule->addEntry(entry);
    gmr.addRootRule(rootRule);
   // auto rule = std::make_shared<StochasticProductionRule>(StochasticNote(StochasticNote::Durations::half));


    return gmr;
}

//static void testGrammarSub(INITFN f)
static void testGrammarSub(const StochasticGrammar* grammar)
{
#if 1
   // GKEY init = f();
    // TODO: put something in this grammar
   //     std::vector<StochasticProductionRulePtr> grammar;



            // TODO: can we make a verion of is grammar valid? I think so
  //  bool b = ProductionRule::isGrammarValid(rules, numRules, init);
  //  assert(b);

    TestEvaluator es(AudioMath::random());
    es.grammar = grammar;

    StochasticProductionRule::evaluate(es, es.grammar->getRootRule());   
    assert(es.getNumSymbols() > 0);
#endif
}

static void testGrammar1() {
    auto grammar = getGrammar();
    testGrammarSub(&grammar);

}


void testStochasticGrammar2() {
    test0();
    testNoteDurations();
    test1();
    testGrammar1();
    
}