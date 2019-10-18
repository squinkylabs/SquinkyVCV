#include "InputScreen.h"
#include "SqGfx.h"
#include "UIPrefs.h"

InputScreen::InputScreen(const ::rack::math::Vec& pos,
        const ::rack::math::Vec& size,
        MidiSequencerPtr seq)
{
    box.pos = pos;
    box.size = size;
    sequencer = seq;    
}

void InputScreen::draw(const Widget::DrawArgs &args)
{
    NVGcontext *vg = args.vg;
    SqGfx::filledRect(vg, UIPrefs::NOTE_EDIT_BACKGROUND, 0, 0, box.size.x, box.size.y);
}


//*********************************************************************

void InputScreenSet::show(::rack::widget::Widget* parent)
{
    ::rack::widget::Widget* screen = screens[0].get();
    parent->addChild(screen);
}

void InputScreenSet::add(InputScreenPtr is)
{
    screens.push_back(is);
}

InputScreenSet::~InputScreenSet()
{
    DEBUG("dtor iss");
}