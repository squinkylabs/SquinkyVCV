#pragma once

#include "SqLog.h"
#include "StochasticNote.h"

#include "asserts.h"
#include <memory>
#include <set>
#include <vector>

class StochasticProductionRuleEntry;
class StochasticProductionRule;
class StochasticGrammar;

using StochasticProductionRuleEntryPtr = std::shared_ptr< StochasticProductionRuleEntry>;
using StochasticProductionRulePtr = std::shared_ptr<StochasticProductionRule>;
using StochasticGrammarPtr = std::shared_ptr<StochasticGrammar>;
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

    bool isValid() const { return probabilty > 0 && probabilty <= 1 && !rhsProducedNotes.empty(); }
    int duration() const;
    void _dump() const;
};

inline int 
StochasticProductionRuleEntry::duration() const {
    int ret = 0;
    for (auto note : rhsProducedNotes) {
        ret += note.duration;
    }
    return ret;
}

inline void 
StochasticProductionRuleEntry::_dump() const {
    SQINFO("Entry p=%f notes:");
    for (auto note : rhsProducedNotes) {
        SQINFO("  note %d", note.duration);
    }
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
        StochasticGrammarPtr grammar;
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

    static bool isGrammarValid(const StochasticGrammar& grammar);
    bool isRuleValid() const;
    void _dump() const;
private:
    static std::vector<StochasticNote>*  _evaluateRule(const StochasticProductionRule& rule, float random);
    std::vector<StochasticProductionRuleEntryPtr> entries;

    static bool isGrammarValidSub(const StochasticGrammar& grammar, StochasticProductionRulePtr nextRule, std::set<size_t> & rulesHit);
};

inline
StochasticProductionRule::StochasticProductionRule(const StochasticNote& n) : lhs(n) {
    assert(lhs.duration > 0);
    SQINFO("ctor of rule, pased %d, have %d", n.duration, lhs.duration);
}

inline void 
StochasticProductionRule::addEntry(StochasticProductionRuleEntryPtr entry) {
    assert(entry->isValid());
    SQINFO("add Entry, dur of ent = %d", entry->duration());
    assertEQ(entry->duration(), lhs.duration);
    entries.push_back(entry);
}


