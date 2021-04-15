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
    auto tempScreen = screen;
    auto tempParent = parent;
    parent = nullptr;
    screen = nullptr;

    if (tempScreen) {
        auto values = tempScreen->getValues();
        if (bOK) {
            tempScreen->execute();
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
        this->dismiss(bOK);
    };

    InputScreenPtr is;
    switch(screenId) {
        case Screens::Invert:
            is = make<XformInvert>(size, seq, dismisser);
            break;
        case Screens::Transpose:
            is = make<XformTranspose>(size, seq, dismisser);
            break;
        case Screens::ReversePitch:
            is = make<XformReversePitch>(size, seq, dismisser);
            break;
        case Screens::ChopNotes:
            is = make<XformChopNotes>(size, seq, dismisser);
            break;
        case Screens::QuantizePitch:
            is = make<XFormQuantizePitch>(size, seq, dismisser);
            break;
        case Screens::MakeTriads:
            is = make<XFormMakeTriads>(size, seq, dismisser);
            break;
        default:
            WARN("no handler for enum %d", int(screenId));
            assert(false);
    }
    screen = is;
    parent->addChild(is.get());
    parentWidget = parent;
}

const char* InputScreenManager::xformName(Screens screen)
{
    const char* ret = nullptr;;
    switch (screen) {
        case Screens::Invert:
            ret = "Invert";
            break;
        case Screens::Transpose:
            ret = "Transpose";
            break;
        case Screens::ReversePitch:
            ret = "Reverse Pitch";
            break;
        case Screens::ChopNotes:
            ret = "Chop Notes";
            break;
        case Screens::QuantizePitch:
            ret = "Quantize Pitch";
            break;
        case Screens::MakeTriads:
            ret = "Make Triads";
            break;
        default:
            WARN("no name for enum %d", int(screen));
            ret = "name";
            assert(false);
    }
    return ret;
}