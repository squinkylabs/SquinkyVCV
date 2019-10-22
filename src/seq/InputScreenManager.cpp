#include "rack.hpp"
#include "InputScreen.h"
#include "InputScreenManager.h"

InputScreenManager::~InputScreenManager()
{
    DEBUG("dtor iss");
   // dismiss();
  // DEBUG("dtor iss 2");
}

/*
const ::rack::math::Vec& pos,
        const ::rack::math::Vec& size,
        MidiSequencerPtr seq,
        std::function<void()> dismisser
    */

 //    InputScreenPtr is = std::make_shared<InputScreen>(Vec(0, 0), box.size, sequencer, dismisser);


void InputScreenManager::show(::rack::widget::Widget* parent, Screens, Callback)
{
   // ::rack::widget::Widget* screen = screens[0].get();
  //  currentScreenIndex = 0;

  // hard code to test screen for now

    InputScreenPtr is = std::make_shared<InputScreen>(::rack::math::Vec(0, 0), size, sequencer, dismisser);
   // ::rack::widget::Widget* screen = new InputScreen();
    parent->addChild(is.get());
    parentWidget = parent;
}