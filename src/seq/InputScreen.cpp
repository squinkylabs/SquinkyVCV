#include "InputScreen.h"
#include "SqGfx.h"
#include "UIPrefs.h"

InputScreen::InputScreen(const ::rack::math::Vec& pos,
        const ::rack::math::Vec& size,
        MidiSequencerPtr seq,
        std::function<void()> _dismisser)
{
    box.pos = pos;
    box.size = size;
    sequencer = seq;   
    this->dismisser = _dismisser; 
    DEBUG("dismisser = %p", _dismisser);
}

InputScreen::~InputScreen()
{
    DEBUG("dtor if input screen");
}
void InputScreen::draw(const Widget::DrawArgs &args)
{
    NVGcontext *vg = args.vg;
    SqGfx::filledRect(vg, UIPrefs::NOTE_EDIT_BACKGROUND, 0, 0, box.size.x, box.size.y);
}

 void InputScreen::onButton(const ::rack::event::Button &e)
 {
     Widget::onButton(e);
    DEBUG("InputScreen::onButton"); fflush(stdout);
    if (dismisser) {
        dismisser();
    } else DEBUG("no dismisser");
    e.consume(this);
 }

//*********************************************************************

void InputScreenSet::show(::rack::widget::Widget* parent)
{
    ::rack::widget::Widget* screen = screens[0].get();
    currentScreenIndex = 0;
    parent->addChild(screen);
    parentWidget = parent;
}

void InputScreenSet::dismiss()
{
    ::rack::widget::Widget* screen = screens[0].get();
    parentWidget->removeChild(screen);
    parentWidget = nullptr;
    currentScreenIndex = 0;
}

void InputScreenSet::add(InputScreenPtr is)
{
    screens.push_back(is);
}

InputScreenSet::~InputScreenSet()
{
    DEBUG("dtor iss");
}