#include "rack.hpp"
#include "InputScreen.h"
#include "XformScreens.h"
#include "InputScreenManager.h"


InputScreenManager::InputScreenManager(::rack::math::Vec siz) : size(siz)
{
}

InputScreenManager::~InputScreenManager()
{
    dismiss(false);
}

void InputScreenManager::dismiss(bool bOK)
{
    DEBUG("In manager dismiis handler (%d)", bOK);

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
        if (bOK) {
            DEBUG("in execute, running xform");
            tempScreen->execute();
        } else {
            DEBUG("in dismiss, skipping due to cancel");
        }
        tempScreen->clearChildren();
        this->callback();
        callback = nullptr;
    }
    if (tempParent) {
        tempParent->removeChild(tempScreen.get());
    }
}


template <class T>
std::shared_ptr<T> InputScreenManager::make(
    const ::rack::math::Vec& size,
    MidiSequencerPtr seq, 
    std::function<void(bool)> dismisser)
{
    return std::make_shared<T>(::rack::math::Vec(0, 0), size, seq, dismisser);
}

void InputScreenManager::show(
    ::rack::widget::Widget* parnt, 
    Screens screenId,  
    MidiSequencerPtr seq,
    Callback cb)
{
    this->callback = cb;
    parent = parnt;
    auto dismisser = [this](bool bOK) {
        DEBUG("in manager::dismisser %d", bOK);
        this->dismiss(bOK);
    };

    DEBUG("about to make input screen size = %f, %f", size.x, size.y);
    InputScreenPtr is;
    switch(screenId) {
        case Screens::Invert:
            is = make<XformInvert>(size, seq, dismisser);
            break;
        case Screens::Transpose:
            is = make<XformTranspose>(size, seq, dismisser);
            break;
        default:
            WARN("no handler for enum %d", int(screenId));
            assert(false);
    }
    screen = is;
    parent->addChild(is.get());
    parentWidget = parent;
}

std::string InputScreenManager::xformName(Screens screen)
{
    std::string ret;
    switch (screen) {
        case Screens::Invert:
            ret = "Invert";
            break;
         case Screens::Transpose:
            ret = "Transpose";
            break;
        default:
            WARN("no name for enum %d", int(screen));
            assert(false);

    }
    return ret;
}