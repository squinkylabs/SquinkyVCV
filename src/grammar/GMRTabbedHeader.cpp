
#include "GMRTabbedHeader.h"

#include "../Squinky.hpp"
#include "../ctrl/SqHelper.h"
#include "../seq/SqGfx.h"
#include "SqLog.h"
#include "TextUtils.h"

GMRTabbedHeader::GMRTabbedHeader() {
    regFont = TextUtils::loadFont(TextUtils::WhichFont::regular);
    boldFont = TextUtils::loadFont(TextUtils::WhichFont::bold);
};

void GMRTabbedHeader::draw(const DrawArgs& args) {
    auto vg = args.vg;

    drawTabText(vg);
    drawLineUnderTabs(vg);

    OpaqueWidget::draw(args);
}

static const float textBaseline = 15;
static const float tabUnderline = textBaseline + 5;
static const float underlineThickness = 1;

static const NVGcolor highlighColor = nvgRGBAf(1, 1, 1, .9);
static const NVGcolor unselectedColor = nvgRGBAf(1, 1, 1, .3);

void GMRTabbedHeader::drawLineUnderTabs(NVGcontext* vg) {
    float x = 0;
    float w = this->box.size.x;
    float y = tabUnderline;
    float h = underlineThickness;
    SqGfx::filledRect(vg, unselectedColor, x, y, w, h);

    x = 7;
    w = 30;
    y = tabUnderline;
    h = underlineThickness;
    SqGfx::filledRect(vg, highlighColor, x, y, w, h);
}

void GMRTabbedHeader::drawTabText(NVGcontext* vg) {
    const int n = 3;
    float x = 10;
    const float y = textBaseline;
    const char* labels[n] = {"Main", "Whole", "Half"};

    for (int i = 0; i < n; ++i) {
        auto color = i == 0 ? highlighColor : unselectedColor;
        const char* text = labels[i];
        int f = (i == 0) ? boldFont->handle : regFont->handle;
        nvgFillColor(vg, color);
        nvgFontFaceId(vg, f);
        nvgFontSize(vg, 12);
        nvgText(vg, x, y, text, nullptr);
        x += 36;
    }
}

void GMRTabbedHeader::onButton(const event::Button& e) {
    if ((e.button != GLFW_MOUSE_BUTTON_LEFT) ||
            (e.action != GLFW_RELEASE)) {
            return;
        }
    int button = e.button;
    float x = e.pos.x;
    float y = e.pos.y;
    SQINFO("button in header, type=%d x=%f y=%f", button, x, y);
}