

#include "ProductionRuleEditor.h"
#include "RuleRowEditor.h"

#include "../seq/SqGfx.h"

#include "../ctrl/SqHelper.h"


ProductionRuleEditor::ProductionRuleEditor(StochasticGrammarPtr g, const StochasticNote& n) : grammar(g), lhs(n) {
     auto row1 = new RuleRowEditor();
     row1->box.pos.x = 10;
     row1->box.pos.y = 20;
     row1->box.size.x = 240;
     row1->box.size.y = 40;
     SQINFO("adding rule row editor with pos=10.20");


     this->addChild(row1);

}

void ProductionRuleEditor::draw(const DrawArgs &args) {
     auto vg = args.vg;
     SqGfx::drawText2(vg, 30, 200, "Production Rules", 18, SqHelper::COLOR_WHITE);
     OpaqueWidget::draw(args);
}