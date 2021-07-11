#pragma once

#include "../Squinky.hpp"

struct NVGcontext;

using FontPtr = std::shared_ptr<Font>;

class GMRTabbedHeader  : public OpaqueWidget {
public:
    GMRTabbedHeader();
    GMRTabbedHeader(const GMRTabbedHeader&) = delete;
    
    void draw(const DrawArgs &args) override;
private:
    FontPtr regFont;
    FontPtr boldFont;

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