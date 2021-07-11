

#include "ProductionRuleEditor.h"

#include "../seq/SqGfx.h"

#include "../ctrl/SqHelper.h"


ProductionRuleEditor::ProductionRuleEditor(StochasticGrammarPtr g) : grammar(g) {

}

void ProductionRuleEditor::draw(const DrawArgs &args) {
     auto vg = args.vg;
     SqGfx::drawText2(vg, 30, 200, "Production Rules", 18, SqHelper::COLOR_WHITE);
}