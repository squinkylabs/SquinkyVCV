
#include "GMRTabbedHeader.h"

#include "../Squinky.hpp"
#include "../ctrl/SqHelper.h"
#include "../seq/SqGfx.h"
#include "SqLog.h"
#include "TextUtils.h"

GMRTabbedHeader::GMRTabbedHeader() {
    regFont = TextUtils::loadFont(TextUtils::WhichFont::regular);
    boldFont = TextUtils::loadFont(TextUtils::WhichFont::bold);

    labels = {"Main", "Whole", "Half", "Quarter"};
    requestLabelPositionUpdate = true;
    //updateLabelPositions();
};

void GMRTabbedHeader::draw(const DrawArgs& args) {
    auto vg = args.vg;

    if (requestLabelPositionUpdate) {
        updateLabelPositions(vg);
        requestLabelPositionUpdate = false;
    }

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
    SQINFO("button in header, type=%d x=%fx, y=%f", button, x, y);
    const int newIndex = x2index(x);
    if (newIndex >= 0) {
        selectNewTab(newIndex);
    }
}

int GMRTabbedHeader::x2index(float x) const {
    SQINFO("x2INdex called with x=%f", x);
    for (int i=0; i< labels.size(); ++ i) {
        SQINFO("at %d %f, %f", i, labelPositions[i].first, labelPositions[i].second);
        if ((x >= labelPositions[i].first) && (x <= (labelPositions[i].first + labelPositions[i].second))) {
            return x;
        }
    }
    //assert(false);
    return -1;
}
//  float index2x(int index) const;
void GMRTabbedHeader::selectNewTab(int index) {
    assert(false);
}

// Measures the specified text string. Parameter bounds should be a pointer to float[4],
// if the bounding box of the text should be returned. The bounds value are [xmin,ymin, xmax,ymax]
// Returns the horizontal advance of the measured text (i.e. where the next character should drawn).
// Measured values are returned in local coordinate space.
//float nvgTextBounds(NVGcontext* ctx, float x, float y, const char* string, const char* end, float* bounds);

static const float leftMargin = 10;
static const float spaceBetweenTabs = 15;
void GMRTabbedHeader::updateLabelPositions(NVGcontext* vg) {
    labelPositions.clear();
    float x = leftMargin;
    for (auto label : labels) {
        float nextX =  nvgTextBounds(vg, x, 0, label.c_str(), nullptr, nullptr);
        float width = nextX - x;
        labelPositions.push_back( std::make_pair(x, width));
        SQINFO("pos = %f, w=%f", x, width);
        x += (width + spaceBetweenTabs);
    }
}