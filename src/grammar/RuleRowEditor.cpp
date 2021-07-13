
#include "RuleRowEditor.h"

#include "../ctrl/SqWidgets.h"
#include "../seq/SqGfx.h"

/*
  addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        //Vec(knobX, knobY + 0 * dy),
        Vec(8, 174),
        module, Comp::ATTACK_PARAM));
        */
RuleRowEditor::RuleRowEditor() {
    SQINFO("*** RuleRowEditor 1");

    // module = std::make_shared<Module>();
    module = new Module();
    this->setModule(module);

    SQINFO("module0: params=%d pq=%d", module->params.size(), module->paramQuantities.size());

    module->config(1, 0, 0, 0);
    SQINFO("module1: params=%d pq=%d", module->params.size(), module->paramQuantities.size());

    const int paramId = 0;
    module->configParam(paramId, 0, 100, 20, "probability");

    SQINFO("module: params=%d pq=%d", module->params.size(), module->paramQuantities.size());

    /*	o->box.pos = pos;
	if (module) {
		o->paramQuantity = module->paramQuantities[paramId];
	}
    */
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