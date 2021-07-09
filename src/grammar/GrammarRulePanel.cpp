
#include "GrammarRulePanel.h"
#include "SqLog.h"

#include "../seq/UIPrefs.h"
#include "../seq/sqGfx.h"
#include "../ctrl/PopupMenuParamWidget.h"

GrammarRulePanel::GrammarRulePanel(const Vec &pos, const Vec &size, StochasticGrammarPtr grammar, Module* m) : module(m) {
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
}

void GrammarRulePanel::draw(const DrawArgs &args) {
    auto vg = args.vg;

    nvgScissor(vg, 0, 0, this->box.size.x, this->box.size.y);
    SqGfx::filledRect(
        vg,
        UIPrefs::NOTE_EDIT_BACKGROUND,
        this->box.pos.x,
        this->box.pos.y,
        this->box.size.x,
        this->box.size.y);
    OpaqueWidget::draw(args);
}