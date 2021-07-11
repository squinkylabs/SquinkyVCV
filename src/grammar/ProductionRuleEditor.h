#pragma once

#include "SqLog.h"
#include "../Squinky.hpp"

class StochasticGrammar;
using StochasticGrammarPtr = std::shared_ptr<StochasticGrammar>;

class ProductionRuleEditor : public OpaqueWidget {
public:
    ProductionRuleEditor(StochasticGrammarPtr);
    ProductionRuleEditor() = delete;
    ProductionRuleEditor(const ProductionRuleEditor&) = delete;
    const ProductionRuleEditor& operator=(const ProductionRuleEditor&) = delete;

    ~ProductionRuleEditor() { SQINFO("dtor of ProductionRuleEditor"); }
    void draw(const DrawArgs &args) override;
private:

    StochasticGrammarPtr grammar;
};