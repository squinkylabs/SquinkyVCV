#pragma once

#include "../Squinky.hpp"      // pulls in rack headers
#include "../ctrl/SqHelper.h"
#include "../seq/sqGfx.h"

class FakeScreen : public OpaqueWidget {
public:
    FakeScreen(const Vec &pos, const Vec &size, bool whichOne);
    void draw(const DrawArgs &args) override;

private:
    const bool which;
};

inline FakeScreen::FakeScreen(const Vec &pos, const Vec &size, bool whichOne) : which(whichOne) {
    this->box.pos = pos;
    this->box.size = size;
}

inline void
FakeScreen::draw(const DrawArgs &args) {
    auto vg = args.vg;

  //  nvgScissor(vg, 0, 0, this->box.size.x, this->box.size.y);

    NVGcolor color = which ? nvgRGB(0xf0, 0, 0) : nvgRGB(0, 0xf0, 0);
    SqGfx::filledRect(
        vg,
        color,
        0,
        0,
        this->box.size.x,
        this->box.size.y);
   

    if (which) {
        SqGfx::drawText2(vg, 5, 15, "RED ONE", 18, SqHelper::COLOR_BLACK);
        SqGfx::drawText2(vg, 5, 40, "RED ONE line 2", 18, SqHelper::COLOR_WHITE);
    } else {
        SqGfx::drawText2(vg, 5, 15, "GREEN ONE", 18, SqHelper::COLOR_SQUINKY);
        SqGfx::drawText2(vg, 5, 40, "GREEN ONE line 2", 18, SqHelper::COLOR_SQUINKY);
    }
     OpaqueWidget::draw(args);
}