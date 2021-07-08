#pragma once

#include "StochasticNote.h"

#include "asserts.h"
#include <memory>
#include <vector>

class StochasticProductionRuleEntry;
class StochasticProductionRule;
class StochasticGrammar;

using StochasticProductionRuleEntryPtr = std::shared_ptr< StochasticProductionRuleEntry>;
using StochasticProductionRulePtr = std::shared_ptr<StochasticProductionRule>;

/**
 * 
 */
class StochasticProductionRuleEntry {
public:
    std::vector<StochasticNote> rhsProducedNotes;
    double probabilty = 0;

    static StochasticProductionRuleEntryPtr make() {
        return std::make_shared<StochasticProductionRuleEntry>();
    }

    bool isValid() const { return probabilty > 0 && probabilty <= 1; }

    // TODO: make this int?
    double duration() const;
};

inline double 
StochasticProductionRuleEntry::duration() const {
    double ret = 0;
    for (auto note : rhsProducedNotes) {
        ret += note.duration;
    }
    return ret;
}

/**
 *
 */
class StochasticProductionRule {
public:
    // this class from the old stuff. may not make sense here.
    class EvaluationState
    {
    public:
        EvaluationState(AudioMath::RandomUniformFunc xr) : r(xr)
        {
        }
        const StochasticGrammar* grammar = nullptr;
        // evaluation state needs access to all the rules
      //  std::vector<StochasticProductionRulePtr>* rules = nullptr;     // from old stuff??
       // int numRules;
        AudioMath::RandomUniformFunc r;		//random number generator to use 
        virtual void writeSymbol(const StochasticNote& sym)
        {
        }
    };

    StochasticProductionRule(const StochasticNote& n);

    StochasticProductionRule() = delete;
    StochasticProductionRule(const StochasticProductionRule&) = delete;
    const StochasticProductionRule& operator = (const StochasticProductionRule&) = delete;
    
    void addEntry(StochasticProductionRuleEntryPtr entry);
    size_t size() const { return entries.size(); }

    // es has all the rules
    // this is that the old one had
    /** 
     * Expands a production rule all the way down to the terminal symbols
     * puts them all into es
     * 
     * @param es has all the stuff in it, including all the rules
     * @param ruleToEval is the index of a prcduction rule in es to expand.
     **/
   // static void evaluate(EvaluationState& es, const StochasticNote& note);
    static void evaluate(EvaluationState& es, const StochasticProductionRulePtr);
    const StochasticNote lhs;
private:
//std::vector<StochasticNote>
 //   static int _evaluateRule(const StochasticProductionRule& rule, float random);
    static std::vector<StochasticNote>*  _evaluateRule(const StochasticProductionRule& rule, float random);
   // const StochasticNote lhs;
    std::vector<StochasticProductionRuleEntryPtr> entries;
};

inline
StochasticProductionRule::StochasticProductionRule(const StochasticNote& n) : lhs(n) {
}

inline void 
StochasticProductionRule::addEntry(StochasticProductionRuleEntryPtr entry) {
    assert(entry->isValid());
    assertEQ(entry->duration(), lhs.duration);
    entries.push_back(entry);
}


