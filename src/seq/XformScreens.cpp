
#include "InputControls.h"
#include "XformScreens.h"


//const NVGcolor TEXT_COLOR = nvgRGB(0xc0, 0xc0, 0xc0);

using Widget = ::rack::widget::Widget;
using Vec = ::rack::math::Vec;
using Label = ::rack::ui::Label;

#if 0
Label* addLabel(Widget* widget, const Vec& v, const char* str, const NVGcolor& color = TEXT_COLOR)
{
    Label* label = new Label();
    label->box.pos = v;
    label->text = str;
    label->color = color;
    widget->addChild(label);
    return label;
}
#endif

//**************************** Invert *********************************
XformInvert::XformInvert(
    const ::rack::math::Vec& pos,
    const ::rack::math::Vec& size,
     MidiSequencerPtr seq,
    std::function<void()> dismisser) : InputScreen(pos, size, seq, dismisser)
{
    // TODO: add label to pitch input
    float x = 100;
    float y = 30;
  //  addLabel(this, Vec(x, y), "Axis" );
 //   y += 30;
    addPitchInput(Vec(x, y), "Axis");
#if 0
    auto pop = new InputPopupMenuParamWidget();
    pop->setLabels( {"first", "second", "third"});
    pop->box.size.x = 76;    // width
    pop->box.size.y = 22;     // should set auto like button does
    pop->setPosition(Vec(100, 50));
    pop->text = "first";
    this->addChild(pop);
    inputControls.push_back(pop);
    #endif
}