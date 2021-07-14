
#include "StochasticProductionRule.h"

#include "SqLog.h"
#include "StochasticGrammar2.h"

#include <set>

std::vector<StochasticNote>* StochasticProductionRule::_evaluateRule(const StochasticProductionRule& rule, float random) {
    assert(random >= 0 && random <= 1);
    assert(&rule != nullptr);

    for (auto it = rule.entries.begin(); it != rule.entries.end(); ++it) {
        StochasticProductionRuleEntryPtr entry = *it;
        if (entry->probability >= random) {
            return &entry->rhsProducedNotes;
        }
    }
    // no rule fired, will terminate
    return nullptr;
}

void StochasticProductionRule::evaluate(EvaluationState& es, const StochasticProductionRulePtr ruleToEval) {
    assert(ruleToEval);
    assert(ruleToEval->isRuleValid());

    auto result = _evaluateRule(*ruleToEval, es.r());
    if (!result)  // request to terminate recursion
    {
        es.writeSymbol(ruleToEval->lhs);
        SQINFO("evaulate found terminal, writing");
    } else {
        SQINFO("expanding %d into %d notes", ruleToEval->lhs.duration, result->size());
        for (auto note : *result) {         
            auto rule = es.grammar->getRule(note);
            if (!rule) {
                SQINFO("found a terminal in the results  %d", note.duration);
                es.writeSymbol(note);
            }
            else {
                evaluate(es, rule);
            }
        }
    }
}

bool StochasticProductionRule::isRuleValid() const {
    if (this->lhs.duration < 1) {
        SQINFO("zero duration rule");
        return false;
    }
    for (auto entry : entries) {
        if (!entry->isValid()) {
            SQINFO("bad entry");
            entry->isValid();
            return false;
        }
        if (entry->duration() != lhs.duration) {
            SQINFO("entry mismatch %d vs %d", lhs.duration, entry->duration());
            return false;
        }
    }

    return true;
}

void StochasticProductionRule::_dump() const {
    assert(this);
    SQINFO("dump rule, lhs dur=%d", this->lhs.duration);
    for (auto entry : this->entries) {
       // StochasticProductionRuleEntryPtr e = entry;
        entry->_dump();
    }
    
}

bool StochasticProductionRule::isGrammarValidSub(const StochasticGrammar& grammar, StochasticProductionRulePtr rule, std::set<size_t>& rulesHit) {
    assert(rule);

    SQINFO("enteer sub rule = %p", rule.get());
    if (!rule->isRuleValid()) {
        SQINFO("invalid rule");
        rule->_dump();
        return false;
    }

   size_t x = (size_t)(void *)rule.get();
   rulesHit.insert(x);
    auto& entries = rule->entries;
    for (auto entry : entries) {
        auto notes = entry->rhsProducedNotes;
        for (auto note : notes) {
            auto nextRule = grammar.getRule(note);
            if (!nextRule) {
                SQINFO("found a note entry with no rule %d. must be a terminal", note.duration);
                return true;
            }
            if (nextRule.get() == rule.get()) {
                SQINFO("grammar has loop");
                return false;
            }
            if (!isGrammarValidSub(grammar, nextRule, rulesHit)) {
                SQINFO("found a bad rule");
                return false;
            }
        }
    }
    return true;
}

bool StochasticProductionRule::isGrammarValid(const StochasticGrammar& grammar) {
    auto nextRule = grammar.getRootRule();
    if (!nextRule) {
        SQINFO("grammar has no root");
        return false;
    }

    std::set<size_t> rulesHit;
    bool ok = isGrammarValidSub(grammar, nextRule, rulesHit);
    if (!ok) {
        return false;
    }

    if (rulesHit.size() != grammar.size()) {
        SQINFO("didn't hit all rules g=%d found%d", grammar.size(), rulesHit);
        return false;
    }

    return true;
}
