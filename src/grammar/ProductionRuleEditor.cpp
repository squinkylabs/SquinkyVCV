

#include "ProductionRuleEditor.h"

#include "../ctrl/SqHelper.h"
#include "../seq/SqGfx.h"
#include "RuleRowEditor.h"
#include "StochasticGrammar2.h"
#include "StochasticProductionRule.h"

ProductionRuleEditor::ProductionRuleEditor(StochasticGrammarPtr g, const StochasticNote& n) : grammar(g), lhs(n) {
    auto ruleForThisScreen = grammar->getRule(lhs);
    auto entries = ruleForThisScreen->getEntries();

    float y = 20;
    for (auto entry : entries) {
        auto row1 = new RuleRowEditor(entry);
        row1->box.pos.x = 10;
        row1->box.pos.y = y;
        row1->box.size.x = 240;
        row1->box.size.y = 40;

        y += 46;
        this->addChild(row1);
    }
}

void ProductionRuleEditor::draw(const DrawArgs& args) {
    auto vg = args.vg;
    const char* text = "Perhaps some UI here for inserting/deleting rules.";
    //  SqGfx::drawText2(vg, 30, 200, text, 12, SqHelper::COLOR_WHITE);
#if 0
    nvgFillColor(vg, SqHelper::COLOR_WHITE);

    nvgFontFaceId(vg, ::rack::appGet()->window->uiFont->handle);
    nvgFontSize(vg, 12);
    nvgTextBox(vg, 30, 200, 200, text, nullptr);
#endif

    SqGfx::drawTextBox(vg, 30, 200, 200, text, 12, SqHelper::COLOR_WHITE);
    OpaqueWidget::draw(args);
}