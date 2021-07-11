#pragma once

#include "StochasticNote.h"

#include <assert.h>

#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

class StochasticProductionRule;

class StochasticGrammar;

using StochasticProductionRulePtr = std::shared_ptr<StochasticProductionRule>;
using StochasticGrammarPtr = std::shared_ptr<StochasticGrammar>;
using ConstStochasticGrammarPtr = std::shared_ptr<const StochasticGrammar>;

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

    std::vector<StochasticNote> getAllLHS() const;

    static StochasticGrammarPtr getDemoGrammar();
    size_t size() const { return rules.size(); }
private:
// DO WE REALLY NEED MULTIMAP? maybe in future?
    // TODO: make unordered work
    //  std::unordered_multimap<StochasticNote, StochasticProductionRulePtr> map;
    std::multimap<StochasticNote, StochasticProductionRulePtr> rules;
    StochasticProductionRulePtr rootRule;
};

