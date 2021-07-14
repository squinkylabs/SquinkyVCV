
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
        assert(test.find(note) == test.end());
        test.insert(note);
        ret.push_back(note);
    }

    return ret;
}

StochasticGrammarPtr StochasticGrammar::getDemoGrammar(DemoGrammar g) {
    StochasticGrammarPtr ret;
    switch (g) {
        case DemoGrammar::simple:
            ret = getDemoGrammarSimple();
            break;
        case DemoGrammar::demo:
            ret = getDemoGrammarDemo();
            break;
    }

    assert(ret);
    return ret;
}

StochasticGrammarPtr StochasticGrammar::getDemoGrammarDemo() {
    auto grammar = std::make_shared<StochasticGrammar>();

    // half rules. splits to quarter
    {
        auto rootRule = std::make_shared<StochasticProductionRule>(StochasticNote::half());

    // half rules
        auto entry = StochasticProductionRuleEntry::make();
        entry->rhsProducedNotes.push_back(StochasticNote::quarter());
        entry->rhsProducedNotes.push_back(StochasticNote::quarter());
        entry->probability = .6;
        rootRule->addEntry(entry);

        assert(rootRule->isRuleValid());
        grammar->addRootRule(rootRule);

        entry = StochasticProductionRuleEntry::make();
        entry->rhsProducedNotes.push_back(StochasticNote::quarter());
        entry->rhsProducedNotes.push_back(StochasticNote::eighth());
        entry->rhsProducedNotes.push_back(StochasticNote::eighth());
        entry->probability = .3;
        rootRule->addEntry(entry);

    }

    // quarter rules. splits to eighth
    {
        auto rule = std::make_shared<StochasticProductionRule>(StochasticNote::quarter());

        auto entry = StochasticProductionRuleEntry::make();
        entry->rhsProducedNotes.push_back(StochasticNote::eighth());
        entry->rhsProducedNotes.push_back(StochasticNote::eighth());
        entry->probability = .7;
        rule->addEntry(entry);
        grammar->addRule(rule);
    }

    // eighth rule
    {
        auto rule = std::make_shared<StochasticProductionRule>(StochasticNote::eighth());

        auto entry = StochasticProductionRuleEntry::make();
        entry->rhsProducedNotes.push_back(StochasticNote::sixteenth());
        entry->rhsProducedNotes.push_back(StochasticNote::sixteenth());
        entry->probability = .5;
        rule->addEntry(entry);
        grammar->addRule(rule);
    }

    return grammar;
}

StochasticGrammarPtr StochasticGrammar::getDemoGrammarSimple() {
    auto grammar = std::make_shared<StochasticGrammar>();

    auto rootRule = std::make_shared<StochasticProductionRule>(StochasticNote::half());
    assert(rootRule->isRuleValid());
    auto entry = StochasticProductionRuleEntry::make();
    entry->rhsProducedNotes.push_back(StochasticNote::quarter());
    entry->rhsProducedNotes.push_back(StochasticNote::quarter());
    entry->probability = .5;
    rootRule->addEntry(entry);
    assert(rootRule->isRuleValid());
    grammar->addRootRule(rootRule);
    return grammar;
}