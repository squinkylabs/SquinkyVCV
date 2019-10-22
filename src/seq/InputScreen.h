#pragma once

#include <functional>
#include <memory>

#include "rack.hpp"

#include <widget/FramebufferWidget.hpp>


class InputControl;
class MidiSequencer;

using InputControlPtr = std::shared_ptr<InputControl>;
using MidiSequencerPtr = std::shared_ptr<MidiSequencer>;

struct InputScreen : public ::rack::widget::OpaqueWidget
{
public:
   InputScreen(const ::rack::math::Vec& pos,
        const ::rack::math::Vec& size,
        std::function<void()> dismisser);
    ~InputScreen();

    
    void draw(const Widget::DrawArgs &args) override;

    /**
     * Mouse handler. For debugging since we don't have buttons yet.
     */
   // void onButton(const ::rack::event::Button &e) override;
private:
   // MidiSequencerPtr sequencer;
    std::function<void()> dismisser = nullptr;
};


using InputScreenPtr = std::shared_ptr<InputScreen>;

#if 0
class InputScreenSet
{
public:
    ~InputScreenSet();
    void add(InputScreenPtr);
    void show(::rack::widget::Widget* parent);

    void dismiss();
private:
    std::vector<InputScreenPtr> screens;
    ::rack::widget::Widget* parentWidget = nullptr;
    int currentScreenIndex = 0;
    bool isDismissing = false;
};

using InputScreenSetPtr = std::shared_ptr<InputScreenSet>;
#endif


