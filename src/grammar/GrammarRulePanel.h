#pragma once

#include "../seq/UIPrefs.h"
#include "../seq/sqGfx.h"
#include "widget/Widget.hpp"

class GrammarRulePanel : public OpaqueWidget {
public:
    GrammarRulePanel(const Vec &pos, const Vec &size) {
        this->box.pos = pos;
        this->box.size = size;
    }
    void draw(const DrawArgs &args) override {
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
};
