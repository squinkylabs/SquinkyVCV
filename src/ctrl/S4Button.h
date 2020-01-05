#pragma once

//#include "Widget.hpp"

class S4Button : public ::rack::OpaqueWidget
{
public:
    S4Button(float size, const Vec& pos);
private:
    FramebufferWidget * fw = nullptr;
};


inline S4Button::S4Button(float size, const Vec& pos)
{
    this->box.size.x = size;
    this->box.size.y = size;
    this->box.pos = pos;
    fw = new FramebufferWidget();
    this->addChild(fw);
}