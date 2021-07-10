
#include "GMRTabbedHeader.h"
#include "TextUtils.h"

#include "../seq/SqGfx.h"

GMRTabbedHeader::GMRTabbedHeader() {
    regFont = TextUtils::loadFont(TextUtils::WhichFont::regular);
    boldFont = TextUtils::loadFont(TextUtils::WhichFont::bold);
   // FontPtr regFont;
  //  FontPtr boldFont;
};

void GMRTabbedHeader::draw(const DrawArgs &args) {
    auto vg = args.vg;
    const NVGcolor color = nvgRGBAf(0, 0, 1, .3);
    SqGfx::filledRect(
        vg,
        color,
        0,
        0,
        this->box.size.x,
        this->box.size.y);

    OpaqueWidget::draw(args);
}
