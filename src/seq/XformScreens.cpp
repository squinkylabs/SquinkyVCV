
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
    float x = 100;
    float y = 30;
    addPitchInput(Vec(x, y), "Axis");
}

void XformInvert::execute()
{
    float p = getAbsPitchFromInput(0);
    WARN("now we need to invert those notes!");
}