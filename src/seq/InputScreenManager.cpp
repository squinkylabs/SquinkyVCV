#include "rack.hpp"
#include "InputScreen.h"
#include "InputScreenManager.h"


InputScreenManager::InputScreenManager(::rack::math::Vec siz) : size(siz)
{
}

InputScreenManager::~InputScreenManager()
{
    DEBUG("dtor iss");
   // dismiss();
  // DEBUG("dtor iss 2");
}

void InputScreenManager::dismiss()
{
    DEBUG("In manager dismiis handler");

    auto tempScreen = screen;
    auto tempParent = parent;
    parent = nullptr;
    screen = nullptr;


    if (tempScreen) {
        auto values = tempScreen->getValues();
        DEBUG("values size = %d", values.size());
        for (auto x : values) {
            DEBUG("value = %.2f", x);
        }
        tempScreen->clearChildren();
    }
    if (tempParent) {
        tempParent->removeChild(tempScreen.get());
    }
}

void InputScreenManager::show(::rack::widget::Widget* parnt, Screens, Callback)
{

  // hard code to test screen for now

    parent = parnt;
    auto dismisser = [this]() {
        DEBUG("in manager::dismisser");
        this->dismiss();
    };

    DEBUG("about to make input screen size = %f, %f", size.x, size.y);
    InputScreenPtr is = std::make_shared<InputScreen>(::rack::math::Vec(0, 0), size, dismisser);
    screen = is;
    parent->addChild(is.get());
    parentWidget = parent;
}