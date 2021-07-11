
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

    TransparentWidget::draw(args);
}

static const float textBaseline = 15;
static const float tabUnderline = textBaseline + 5;
static const float underlineThickness = 1;

static const float leftMargin = 10;
static const float spaceBetweenTabs = 8;

static const NVGcolor highlighColor = nvgRGBf(1, 1, 1);
static const NVGcolor unselectedColor = nvgRGBAf(1, 1, 1, .3);

void GMRTabbedHeader::drawLineUnderTabs(NVGcontext* vg) {
  
    float x = 0;
    float w = this->box.size.x;
    float y = tabUnderline;
    float h = underlineThickness;
    #if 0
    SqGfx::filledRect(vg, unselectedColor, x, y, w, h);
    #endif

   // SQINFO("make draw line aware of size");
    auto pos = labelPositions[currentTab];
    x = pos.first;
    w = pos.second; // - spaceBetweenTabs;
    y = tabUnderline;
    h = underlineThickness;
    SqGfx::filledRect(vg, highlighColor, x, y, w, h);
}

void GMRTabbedHeader::drawTabText(NVGcontext* vg) {
    const float y = textBaseline;

    for (int i = 0; i < int(labels.size()); ++i) {
        const auto pos = labelPositions[i];
        auto color = (i == currentTab) ? highlighColor : unselectedColor;
        const char* text = labels[i].c_str();
     //   int f = (i == currentTab) ? boldFont->handle : regFont->handle;
        int f = regFont->handle;
        nvgFillColor(vg, color);
        nvgFontFaceId(vg, f);
        nvgFontSize(vg, 12);
        nvgText(vg, pos.first, y, text, nullptr);
      //  x += 36;
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
    SQINFO("x2index ret %d", newIndex);
    if (newIndex >= 0) {
        selectNewTab(newIndex);
    }
}

int GMRTabbedHeader::x2index(float x) const {
    SQINFO("x2INdex called with x=%f", x);
    for (int i=0; i< int(labels.size()); ++ i) {
        SQINFO("at %d %f, %f", i, labelPositions[i].first, labelPositions[i].second);
        if ((x >= labelPositions[i].first) && (x <= (labelPositions[i].first + labelPositions[i].second))) {
            return i;
        }
    }
    //assert(false);
    return -1;
}

void GMRTabbedHeader::selectNewTab(int index) {
    if (currentTab != index) {
        currentTab = index;
        if (theCallback) {
            theCallback(index);
        }
    }
}

// Measures the specified text string. Parameter bounds should be a pointer to float[4],
// if the bounding box of the text should be returned. The bounds value are [xmin,ymin, xmax,ymax]
// Returns the horizontal advance of the measured text (i.e. where the next character should drawn).
// Measured values are returned in local coordinate space.
//float nvgTextBounds(NVGcontext* ctx, float x, float y, const char* string, const char* end, float* bounds);


void GMRTabbedHeader::updateLabelPositions(NVGcontext* vg) {
    labelPositions.clear();
    float x = leftMargin;

    //for (auto label : labels) {
    for (int i=0; i< int(labels.size()); ++ i) {

       // int f = (i == currentTab) ? boldFont->handle : regFont->handle;
        int f = regFont->handle;
        nvgFontFaceId(vg, f);
        nvgFontSize(vg, 12);
        SQINFO("about to cal next x with x=%f label=%s", x, labels[i].c_str());

        const float width = nvgTextBounds(vg, 1, 1, labels[i].c_str(), NULL, NULL);
        labelPositions.push_back( std::make_pair(x, width));

        SQINFO("pos = %f, w=%f nextx=%f", x, width);
        assert(width > 0);
       //x += (width + spaceBetweenTabs);
       x = x + width + spaceBetweenTabs;
       SQINFO("at bootom of loop, x set to %f", x);
    }
}

void GMRTabbedHeader::registerCallback(Callback cb) {
    assert(!theCallback);
    theCallback = cb;               // ATM we only support one client.
}