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
    entry->rhsProducedNotes.push_back(StochasticNote::eighth());
    entry->rhsProducedNotes.push_back(StochasticNote::eighth());
    entry->probabilty = .1;
    assert(entry->isValid());


    StochasticProductionRule rule(StochasticNote::quarter());
    rule.addEntry(entry);
    assert(rule.isRuleValid());
}


class TestEvaluator : public StochasticProductionRule::EvaluationState
{
public:
    TestEvaluator(AudioMath::RandomUniformFunc xr) : StochasticProductionRule::EvaluationState(xr)
    {
    }

    void writeSymbol(const StochasticNote& sym) override
    {
        notes.push_back(sym);
    }

    size_t getNumSymbols()
    {
        return notes.size();
    }
private:
    std::vector<StochasticNote> notes;
};



static StochasticGrammar getGrammar() {
    StochasticGrammar gmr;

    auto rootRule = std::make_shared<StochasticProductionRule>(StochasticNote::half());
    assert(rootRule->isRuleValid());
    auto entry = StochasticProductionRuleEntry::make();
    entry->rhsProducedNotes.push_back(StochasticNote::quarter());
    entry->rhsProducedNotes.push_back(StochasticNote::quarter());
    entry->probabilty = .5;
    rootRule->addEntry(entry);
    assert(rootRule->isRuleValid());
    gmr.addRootRule(rootRule);
   // auto rule = std::make_shared<StochasticProductionRule>(StochasticNote(StochasticNote::Durations::half));


    return gmr;
}

//static void testGrammarSub(INITFN f)
static void testGrammarSub(const StochasticGrammar* grammar)
{
    SQINFO("--- test Grammar sub ----");
   bool b = StochasticProductionRule::isGrammarValid(*grammar);
   assert(b);

    TestEvaluator es(AudioMath::random());
    es.grammar = grammar;

    SQINFO("--- test Grammar sub evaluating----");
    StochasticProductionRule::evaluate(es, es.grammar->getRootRule());   
    assert(es.getNumSymbols() > 0);
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