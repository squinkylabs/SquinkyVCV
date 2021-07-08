
#include "StochasticProductionRule.h"

#include "SqLog.h"
#include "StochasticGrammar2.h"

//  static std::vector<StochasticNote>&  _evaluateRule(const StochasticProductionRule& rule, float random);
std::vector<StochasticNote>* StochasticProductionRule::_evaluateRule(const StochasticProductionRule& rule, float random) {
    assert(random >= 0 && random <= 1);

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
    } else {
        //printf("production rule #%d expanded to %d\n", ruleToEval, result);

        // need to expand,then eval all of the expanded codes

        //  GKEY buffer[ProductionRuleKeys::bufferSize];
        // TOOD: reasonable size
        //  StochasticNote buffer[10];

        //  ProductionRuleKeys::breakDown(result, buffer);
        //   for (GKEY * p = buffer; *p != sg_invalid; ++p) {
        //
        for (auto note : *result) {
            //printf("expanding rule #%d with %d\n", ruleToEval, *p);
            evaluate(es, es.grammar->getRule(note));
        }
        //printf("done expanding %d\n
    }
}

bool StochasticProductionRule::isRuleValid() const {
    if (this->lhs.duration < 1) {
        SQINFO("zero duration rule");
        return false;
    }
    assert(false);
    return false;
}

void StochasticProductionRule::_dump() const {
    assert(this);
    SQINFO("dump rule, lhs dur=%d", this->lhs.duration);
    for (auto entry : this->entries) {
       // StochasticProductionRuleEntryPtr e = entry;
        entry->_dump();
    }
    
}

bool StochasticProductionRule::isGrammarValidSub(const StochasticGrammar& grammar, StochasticProductionRulePtr rule, int& rulesHit) {
    assert(rule);

    SQINFO("enteer sub rule = %p", rule.get());
    if (!rule->isRuleValid()) {
        SQINFO("invalid rule");
        rule->_dump();
        return false;
    }
    rulesHit++;
    auto& entries = rule->entries;
    for (auto entry : entries) {
        auto notes = entry->rhsProducedNotes;
        for (auto note : notes) {
            auto nextRule = grammar.getRule(note);
            if (!nextRule) {
                SQINFO("found a note with no rule");
                return false;
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

    int rulesHit = 1;
    bool ok = isGrammarValidSub(grammar, nextRule, rulesHit);
    if (!ok) {
        return false;
    }

    if (rulesHit != grammar.size()) {
        SQINFO("didn't hit all rules");
        return false;
    }

    return true;
}
