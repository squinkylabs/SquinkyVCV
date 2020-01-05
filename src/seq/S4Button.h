#pragma once

//#include "Widget.hpp"

#include "SqGfx.h"
#include "UIPrefs.h"

class S4ButtonDrawer : public ::rack::OpaqueWidget
{
public:
    S4ButtonDrawer(const Vec& size, const Vec& pos)
    {
        this->box.size=size;
    }
    void draw(const DrawArgs &args) override;

};


inline void S4ButtonDrawer::draw(const DrawArgs &args)
{
    SqGfx::filledRect(
                args.vg,
                UIPrefs::NOTE_COLOR,
                this->box.pos.x, box.pos.y, box.size.x, box.size.y); 
                //x, y, width, noteHeight);
}


class S4Button : public ::rack::OpaqueWidget
{
public:
    S4Button(const Vec& size, const Vec& pos);
private:
    FramebufferWidget * fw = nullptr;
    S4ButtonDrawer * drawer = nullptr;
};


inline S4Button::S4Button(const Vec& size, const Vec& pos)
{
    this->box.size = size;
    this->box.pos = pos;
    fw = new FramebufferWidget();
    this->addChild(fw);

    drawer = new S4ButtonDrawer(size, pos);
    fw->addChild(drawer);

    // now need to add child to fw


    //fw->fbSize = this->box.size;
    //DEBUG("made fw, size = %f,%f", fw->fbSize.x, fw->fbSize.y); 
}