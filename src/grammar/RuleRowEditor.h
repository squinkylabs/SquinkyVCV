#pragma once

#include "SqLog.h"
#include "../Squinky.hpp"

class StochasticProductionRuleEntry;
using StochasticProductionRuleEntryPtr = std::shared_ptr<StochasticProductionRuleEntry>;

class RuleRowEditor : public ModuleWidget {
public:
    RuleRowEditor(StochasticProductionRuleEntryPtr entry);
    void draw(const DrawArgs &args) override;
private:
    // We don't own this module's lifetime - VCV will destroy it.
    Module* module = nullptr;
    StochasticProductionRuleEntryPtr entry;
    std::string ruleText;
};