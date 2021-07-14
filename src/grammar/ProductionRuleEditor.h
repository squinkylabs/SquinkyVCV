#pragma once

#include "StochasticNote.h"
#include "../Squinky.hpp"
#include "SqLog.h"


class StochasticGrammar;
using StochasticGrammarPtr = std::shared_ptr<StochasticGrammar>;

class ProductionRuleEditor : public OpaqueWidget {
public:
    ProductionRuleEditor(StochasticGrammarPtr, const StochasticNote& lhs);
    ProductionRuleEditor() = delete;
    ProductionRuleEditor(const ProductionRuleEditor&) = delete;
    const ProductionRuleEditor& operator=(const ProductionRuleEditor&) = delete;

    ~ProductionRuleEditor() { SQINFO("dtor of ProductionRuleEditor"); }
    void draw(const DrawArgs& args) override;

private:
    StochasticGrammarPtr grammar;
    StochasticNote lhs;
};