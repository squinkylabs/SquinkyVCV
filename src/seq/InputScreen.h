#pragma once

#include <memory>

#include "Rack.hpp"

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
        MidiSequencerPtr seq);

    
    void draw(const Widget::DrawArgs &args) override;

private:
    MidiSequencerPtr sequencer;
};

using InputScreenPtr = std::shared_ptr<InputScreen>;

class InputScreenSet
{
public:
    ~InputScreenSet();
    void add(InputScreenPtr);
    void show(::rack::widget::Widget* parent);
private:
    std::vector<InputScreenPtr> screens;
};

using InputScreenSetPtr = std::shared_ptr<InputScreenSet>;

