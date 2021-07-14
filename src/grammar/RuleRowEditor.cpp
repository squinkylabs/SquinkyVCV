
#include "StochasticProductionRule.h"
#include "RuleRowEditor.h"


#include "../ctrl/SqWidgets.h"
#include "../seq/SqGfx.h"


RuleRowEditor::RuleRowEditor(StochasticProductionRuleEntryPtr e) : entry(e) {
  //  SQINFO("ctor prob =%f", prob);
    module = new Module();
    this->setModule(module);
    module->config(1, 0, 0, 0);

    const int paramId = 0;
    const float prob = entry->probability;

    module->configParam(paramId, 0, 100, prob * 100, "probability");

    auto p = new Blue30Knob();
    p->box.pos.x = 150;
    p->box.pos.y = 20;
    p->paramQuantity = module->paramQuantities[paramId];
    SQINFO("*** RuleRowEditor 3");

    //::rack::createParam<T>(pos, module, paramId);
    addParam(p);

    SQINFO("*** RuleRowEditor ctor sizeof this module =%d", sizeof(RuleRowEditor));
}

void RuleRowEditor::draw(const DrawArgs &args) {
    //SQINFO("RuleRowEditor in draw h=%f w=%f x = %f y=%f", this->box.size.y,this->box.size.x, this->box.pos.x, this->box.pos.y);
    OpaqueWidget::draw(args);
}