#pragma once

#include "StochasticNote.h"

#include "asserts.h"
#include <memory>
#include <vector>

class StochasticProductionRuleEntry;
using StochasticProductionRuleEntryPtr = std::shared_ptr< StochasticProductionRuleEntry>;

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
    double duration() const;
};

inline double 
StochasticProductionRuleEntry::duration() const {
    double ret = 0;
    for (auto note : rhsProducedNotes) {
        ret += note.timeDuration();
    }
    return ret;
}

/**
 *
 */
class StochasticProductionRule {
public:
    StochasticProductionRule(const StochasticNote& n);

    StochasticProductionRule() = delete;
    StochasticProductionRule(const StochasticProductionRule&) = delete;
    const StochasticProductionRule& operator = (const StochasticProductionRule&) = delete;
    
    void addEntry(StochasticProductionRuleEntryPtr entry);
    size_t size() const { return entries.size(); }

private:
    const StochasticNote lhs;
    std::vector<StochasticProductionRuleEntryPtr> entries;
};

inline
StochasticProductionRule::StochasticProductionRule(const StochasticNote& n) : lhs(n) {
}

inline void 
StochasticProductionRule::addEntry(StochasticProductionRuleEntryPtr entry) {
    assert(entry->isValid());
    assertEQ(entry->duration(), lhs.timeDuration());
    entries.push_back(entry);
}
