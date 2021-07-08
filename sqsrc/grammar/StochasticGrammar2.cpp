
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