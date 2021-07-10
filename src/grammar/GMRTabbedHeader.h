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

};