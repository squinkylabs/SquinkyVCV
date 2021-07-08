
#include "StochasticProductionRule.h"

#include "SqLog.h"
#include "StochasticGrammar2.h"

#include <set>

std::vector<StochasticNote>* StochasticProductionRule::_evaluateRule(const StochasticProductionRule& rule, float random) {
    assert(random >= 0 && random <= 1);
    assert(&rule != nullptr);

    for (auto it = rule.entries.begin(); it != rule.entries.end(); ++it) {
        //printf("prob[%d] is %d\n", i,  rule.entries[i].probability);
        StochasticProductionRuleEntryPtr entry = *it;
        if (entry->probabilty >= random) {
            //printf("rule fired on code abs val=%d\n", code);
            return &entry->rhsProducedNotes;
        }
    }
    // no rule fired, will terminate
    return nullptr;
}

//  static void evaluate(EvaluationState& es, int ruleToEval);
void StochasticProductionRule::evaluate(EvaluationState& es, const StochasticProductionRulePtr ruleToEval) {
    assert(ruleToEval);
    // copied over stuff from old one
    //printf("\n evaluate called on rule #%d\n", ruleToEval);
    //  StochasticProductionRulePtr rule = (*es.rules)[ruleToEval];
    // StochasticProductionRulePtr rule = es.grammar->getRule(ruleToEval);

#ifdef _MSC_VER
    // TODO: implement is valid?
    //  assert(rule._isValid(ruleToEval));
#endif
    //
    // _evaluateRule(*rule, es.r());
    //  GKEY result = _evaluateRule(rule, es.r());
    auto result = _evaluateRule(*ruleToEval, es.r());
    if (!result)  // request to terminate recursion
    {
        //  GKEY code = ruleToEval;		// our "real" terminal code is our table index
        //printf("production rule #%d terminated\n", ruleToEval);
        //printf("rule terminated! execute code %s\n",  ProductionRuleKeys::toString(code));

        //  assert(false);  /// what do we do now? below is old
        es.writeSymbol(ruleToEval->lhs);
        SQINFO("evaulate found terminal, writing");
    } else {
        //printf("production rule #%d expanded to %d\n", ruleToEval, result);

        // need to expand,then eval all of the expanded codes

        //  GKEY buffer[ProductionRuleKeys::bufferSize];
        // TOOD: reasonable size
        //  StochasticNote buffer[10];

        //  ProductionRuleKeys::breakDown(result, buffer);
        //   for (GKEY * p = buffer; *p != sg_invalid; ++p) {
        //
        SQINFO("expanding %d into %d notes", ruleToEval->lhs.duration, result->size());
        for (auto note : *result) {
            //printf("expanding rule #%d with %d\n", ruleToEval, *p);
         
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
   // rulesHit++;
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
   // int rulesHit = 1;
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
