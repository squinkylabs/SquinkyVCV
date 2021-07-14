#include "GMRScreenHolder.h"

#include "GMRMainScreen.h"
#include "GMRTabbedHeader.h"
#include "ProductionRuleEditor.h"
#include "StochasticGrammar2.h"
#include "SqLog.h"

const float headerHeight = 24;

GMRScreenHolder::GMRScreenHolder(const Vec &pos, const Vec &size) {
    this->box.pos = pos;
    this->box.size = size;
    SQINFO("screen holder pos = %f height=%f", pos.y, size.y);

    grammar = StochasticGrammar::getDemoGrammar(StochasticGrammar::DemoGrammar::demo);

    // First, make the header and set it up
    auto header = new GMRTabbedHeader(grammar);
    header->box.pos.x = 0;
    header->box.pos.y = 0;
    header->box.size.x = this->box.size.x;
    header->box.size.y = headerHeight;
    this->addChild(header);
    // Vec pos2(40, 10);

    // Capturing `this` is a sin. But here we are relying on
    // VCV for memory management. It's fine (if we are careful).
    header->registerCallback([this](int index) {
        //SQINFO("header callback %d", index);
        this->onNewTab(index);
    });

    // Second: set up the main screen as the active child
    Widget *child = new GMRMainScreen(grammar);
    sizeChild(child);
    screens.push_back(child);
    addChild(child);

    SQINFO("main = %f height=%f", child->box.pos.y, child->box.size.y);
}

GMRScreenHolder::~GMRScreenHolder() {
    SQINFO("dtor of GMRScreenHolder");
    for (int i=0; i< int(screens.size()); ++i) {
        // one of the screens is on the stage, and will get killed anyway
        if (i != currentTab) {
            delete screens[i];
        }
    }
}

void GMRScreenHolder::sizeChild(Widget *child) {
    child->box.pos.x = 0;
    child->box.pos.y = headerHeight;
    child->box.size.x = this->box.size.x;
    child->box.size.y = this->box.size.y - headerHeight;

    SQINFO("size child, leave = %f height=%f", child->box.pos.y, child->box.size.y);
}

void GMRScreenHolder::onNewTab(int index) {
    // This should never happen. but just in case
    if (index == currentTab) {
        return;
    }

    // make sure we have enough spaces in array
    if (int(screens.size()) < (index + 1)) {
        screens.resize(index + 1);
        assert(screens[index] == nullptr);
    }

    // Make sure the entry for next screen exists
    if (screens[index] == nullptr) {
        auto allLHS = grammar->getAllLHS();
        assert(index > 0);
        Widget *newScreen = new ProductionRuleEditor(grammar, allLHS[index - 1]);
        sizeChild(newScreen);
         SQINFO("onnewtab makde new one = %f height=%f", newScreen->box.pos.y, newScreen->box.size.y);
        screens[index] = newScreen;
    }

    // remove the old screen
    auto currentScreen = screens[currentTab];
    assert(currentScreen);
    this->removeChild(currentScreen);

    // and add the new one.
    currentScreen = screens[index];
    currentTab = index;
    this->addChild(currentScreen);
}

// TODO: do we need to override draw at all?
void GMRScreenHolder::draw(const DrawArgs &args) {
    auto vg = args.vg;

    nvgScissor(vg, 0, 0, this->box.size.x, this->box.size.y);
    OpaqueWidget::draw(args);
}

#if 0  // old experiment, still has useful transition work
void
GMRScreenHolder::draw(const DrawArgs &args) {
    auto vg = args.vg;


    nvgScissor(vg, 0, 0, this->box.size.x, this->box.size.y);

    childPos += .002;
    if (childPos > 1) {
        childPos = 0;
    }
    //
    float width = this->box.size.x;
  //  float x = 2 * (-width/2 + childPos * width);
  //  float x2 = x + width;
    const float x1 = (childPos -1) * width;
    const float x2 = childPos * width;

    const float ch1ClipX = width * (1 - childPos);
    const float ch1ClipW = width * childPos;

    const float ch2ClipX = 0;
    const float ch2ClipW = width * (1 - childPos);
#if 0
    {
        static int count = 0;
        if (!count) {
            SQINFO("x=%f %f", x1, x2);
            ++count;
            if (count > 20) {
                count = 0;
            }
        }
    }
#endif

    nvgSave(vg);
    nvgTranslate(vg, x1, 0);
    nvgScissor(vg, ch1ClipX, 0, ch1ClipW, this->box.size.y);
    child1->draw(args);
    nvgRestore(vg);

    nvgSave(vg);
    nvgTranslate(vg, x2, 0);
    nvgScissor(vg, ch2ClipX, 0, ch2ClipW, this->box.size.y);
    child2->draw(args);
    nvgRestore(vg);

    OpaqueWidget::draw(args);
}
#endif