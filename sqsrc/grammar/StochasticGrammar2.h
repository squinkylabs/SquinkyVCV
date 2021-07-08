#pragma once

#include <assert.h>

#include <map>
#include <memory>
#include <unordered_map>

class StochasticProductionRule;
class StochasticNote;

using StochasticProductionRulePtr = std::shared_ptr<StochasticProductionRule>;
/**
 * class that holds an entire grammar
 */
class StochasticGrammar {
public:
    StochasticProductionRulePtr getRule(const StochasticNote&) const;

    StochasticProductionRulePtr getRootRule() const;

    void addRule(StochasticProductionRulePtr rule);

    void addRootRule(StochasticProductionRulePtr rule) {
        assert(!rootRule);
        rootRule = rule;
        addRule(rule);
    }

private:
    // TODO: make unordered work
    //  std::unordered_multimap<StochasticNote, StochasticProductionRulePtr> map;
    std::multimap<StochasticNote, StochasticProductionRulePtr> rules;
    StochasticProductionRulePtr rootRule;
};