
#include "StochasticGrammar2.h"
#include "StochasticProductionRule.h"


StochasticProductionRulePtr StochasticGrammar::getRule(const StochasticNote& n) const {
    auto it = rules.find(n);
    return (it == rules.end()) ? nullptr : it->second;
}

StochasticProductionRulePtr StochasticGrammar::getRootRule() const {
    return rootRule;
}

void StochasticGrammar::addRule(StochasticProductionRulePtr rule) {
    const StochasticNote& n = rule->lhs;
    assert(getRule(n) == nullptr);

    std::pair<StochasticNote, StochasticProductionRulePtr> value(n, rule);
    rules.insert(value);
}

std::vector<StochasticNote> StochasticGrammar::getAllLHS() const {
    std::vector<StochasticNote> ret;

    std::set<StochasticNote> test;
    for (auto rule : rules) {
        StochasticNote note = rule.first;
        assert( test.find(note) == test.end());
        test.insert(note);
        ret.push_back(note);
    }
    
    return ret;
}

StochasticGrammarPtr StochasticGrammar::getDemoGrammar() {
    // TODO: should this be a singleton?
    auto grammar = std::make_shared<StochasticGrammar>();

    auto rootRule = std::make_shared<StochasticProductionRule>(StochasticNote::half());
    assert(rootRule->isRuleValid());
    auto entry = StochasticProductionRuleEntry::make();
    entry->rhsProducedNotes.push_back(StochasticNote::quarter());
    entry->rhsProducedNotes.push_back(StochasticNote::quarter());
    entry->probabilty = .5;
    rootRule->addEntry(entry);
    assert(rootRule->isRuleValid());
    grammar->addRootRule(rootRule);
    return grammar;
}