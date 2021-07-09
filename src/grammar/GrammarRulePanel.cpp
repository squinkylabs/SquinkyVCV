
#include "GrammarRulePanel.h"

#include "../ctrl/PopupMenuParamWidget.h"
#include "../ctrl/SqHelper.h"
#include "../seq/UIPrefs.h"
#include "../seq/sqGfx.h"
#include "SqLog.h"
#include "SqStream.h"
#include "StochasticGrammar2.h"
#include "StochasticProductionRule.h"

GrammarRulePanel::GrammarRulePanel(const Vec &pos, const Vec &size, StochasticGrammarPtr g, Module *m) : grammar(g),
                                                                                                         module(m) {
    this->box.pos = pos;
    this->box.size = size;

    const auto wpos = Vec(20, 20);
    int paramId = 0;
    // we need to set up the param stuff to use (abuse) this widget.
    PopupMenuParamWidget *p = createParam<PopupMenuParamWidget>(wpos, m, paramId);

    p->box.size.x = 50;  // width
    p->box.size.y = 22;
    p->box.pos.x = 20;
    p->box.pos.y = 20;

    p->text = "LP";
    p->setLabels({"LP", "BP", "HP", "N"});
    p->setNotificationCallback([](int index) {
        SQINFO("notification callback %d", index);
    });
    addChild(p);

    if (grammar) {
        SQINFO("getting root");
        StochasticProductionRulePtr root = grammar->getRootRule();
        if (root) {
            SQINFO("using root to get dur");
            ruleDuration = root->lhs.duration;
        } else
            SQINFO("no root");
    } else
        SQINFO("no grammar");
}

void GrammarRulePanel::draw(const DrawArgs &args) {
    auto vg = args.vg;

    nvgScissor(vg, 0, 0, this->box.size.x, this->box.size.y);

    SqGfx::filledRect(
        vg,
        UIPrefs::NOTE_EDIT_BACKGROUND,
        0,
        0,
        this->box.size.x,
        this->box.size.y);
#if 0
    SqGfx::filledRect(
        vg,
        UIPrefs::NOTE_COLOR,
        0, 0,
        10, 10);
#endif

    SqStream str;
    str.add(ruleDuration);
    SqGfx::drawText2(vg, 5, 15, str.str().c_str(), 14, SqHelper::COLOR_WHITE);
    OpaqueWidget::draw(args);
}