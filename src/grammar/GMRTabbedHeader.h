#pragma once

#include "../Squinky.hpp"
#include "SqLog.h"
#include <functional>

struct NVGcontext;
class StochasticGrammar;

using FontPtr = std::shared_ptr<Font>;
using StochasticGrammarPtr = std::shared_ptr<StochasticGrammar>;

class GMRTabbedHeader  : public OpaqueWidget {
public:
    GMRTabbedHeader(StochasticGrammarPtr);
    GMRTabbedHeader() = delete;
    GMRTabbedHeader(const GMRTabbedHeader&) = delete;
    ~GMRTabbedHeader() { SQINFO("dtor of GMRTabbedHeader"); }

    using Callback = std::function<void(int index)>;
    void registerCallback(Callback);
    
    void draw(const DrawArgs &args) override;
private:
    FontPtr regFont;
    FontPtr boldFont;
    Callback theCallback;
    StochasticGrammarPtr grammar;

    void drawLineUnderTabs(NVGcontext *);
    void drawTabText(NVGcontext *);

    void onButton(const event::Button& e) override;

    std::vector<std::string> labels;

    // position is x, width
    std::vector<std::pair<float, float>> labelPositions;
    int currentTab = 0;
    bool requestLabelPositionUpdate = false;

    int x2index(float x) const;
    float index2x(int index) const;
    void selectNewTab(int index);
    void updateLabelPositions(NVGcontext *);
};