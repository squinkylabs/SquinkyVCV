#pragma once

#include "../seq/SqGfx.h"
#include "../seq/UIPrefs.h"
#include "widget/TransparentWidget.hpp"

class SqVuMeter : public widget::TransparentWidget
{
public:
    void draw(const DrawArgs &args) override;
    void setGetter( std::function<float(void)> getter);
private:
    std::function<float(void)> lambda = nullptr;
};

inline void SqVuMeter::setGetter( std::function<float(void)> getter)
{
    lambda = getter;
}

inline void SqVuMeter::draw(const DrawArgs &args)
{
    float x = 0;
    float y = 0;
    float width = this->box.size.x;
    float height = this->box.size.y;

    const float dbPerSegment = 2;
    const int numSegments = 8;
    const float xPerSegment = width / numSegments;
    const float segmentWidth = xPerSegment * .8;  

    assert(lambda);
    float atten = lambda();

    for (int seg = 0; seg < numSegments; ++seg) {
        const float segX = x + seg * xPerSegment;
        const int segRtoL = numSegments - (seg + 1);
        float attenThisSegment = dbPerSegment/2  + segRtoL * dbPerSegment;
        const auto color = (atten >= attenThisSegment) ? UIPrefs::VU_ACTIVE_COLOR : UIPrefs::VU_INACTIVE_COLOR;
        SqGfx::filledRect(
            args.vg,
            color,
            segX, y, segmentWidth, height);
        // fprintf(stderr, "segment[%d] x=%f, width=%f\n", seg, segX, segmentWidth);
        }
    TransparentWidget::draw(args);
}