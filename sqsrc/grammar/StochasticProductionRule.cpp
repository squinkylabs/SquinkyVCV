
#include "StochasticProductionRule.h"

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