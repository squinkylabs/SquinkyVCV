
#include "InputControls.h"
#include "XformScreens.h"

using Vec = ::rack::math::Vec;
XformInvert::XformInvert(
    const ::rack::math::Vec& pos,
    const ::rack::math::Vec& size,
    std::function<void()> dismisser) : InputScreen(pos, size, dismisser)
{
    auto pop = new InputPopupMenuParamWidget();
    pop->setLabels( {"first", "second", "third"});
    pop->box.size.x = 76;    // width
    pop->box.size.y = 22;     // should set auto like button does
    pop->setPosition(Vec(100, 50));
    pop->text = "first";
    this->addChild(pop);
    inputControls.push_back(pop);
}