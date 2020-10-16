#pragma once

#include "../seq/SqGfx.h"
#include "../seq/UIPrefs.h"
#include "widget/TransparentWidget.hpp"

class SqVuMeter : public widget::TransparentWidget
{
public:
    void draw(const DrawArgs &args) override;
private:
};

inline void SqVuMeter::draw(const DrawArgs &args)
{
    //TransparentWidget::draw();
   
   // float x = this->box.pos.x;
   // float y = this->box.pos.y;
    float x = 0;
    float y = 0;
    float width = this->box.size.x;
    float height = this->box.size.y;

     //fprintf(stderr, "paint  = %f,%f, siz=%f, %f\n", x, y, width, height);
    SqGfx::filledRect(
        args.vg,
        UIPrefs::NOTE_COLOR,
        x, y, width, height);
    TransparentWidget::draw(args);
}