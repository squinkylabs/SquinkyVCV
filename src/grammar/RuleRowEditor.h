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
    // after construct, put module here.
    // This object will own it.
    // but module widget dtor will delete it anyway - make this a raw pointer..
   // std::shared_ptr<Module> module;
    Module* module = nullptr;
    StochasticProductionRuleEntryPtr entry;

};