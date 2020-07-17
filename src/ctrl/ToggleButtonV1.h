#pragma once

#include "ToggleManager2.h"

/**
 * An SvgButton, but in stead of being momentary it toggles between values.
 */
class SqSvgToggleButton : public ::rack::app::SvgButton
{
public:
    SqSvgToggleButton (::rack::widget::Widget* = nullptr);
    void onDragStart(const event::DragStart &e) override;
    void onDragEnd(const event::DragEnd &e) override;
    void onDragDrop(const event::DragDrop &e) override;

    float getValue() const;
    void setValue(float);
private:
    int index = 0;
    void setIndex(int i);
    ::rack::widget::Widget* actionDelegate = nullptr;
};

inline SqSvgToggleButton::SqSvgToggleButton (::rack::widget::Widget* delegate)
{
    actionDelegate = delegate;
}

inline void SqSvgToggleButton::setIndex(int i)
{
    index = i;
    sw->setSvg(frames[index]);
    fb->dirty = true;
}


inline float SqSvgToggleButton::getValue() const
{
    return std::round(index);
}


inline void SqSvgToggleButton::setValue(float v) 
{
    const int newIndex = int(std::round(v));
    setIndex(newIndex);
}

inline void SqSvgToggleButton::onDragStart(const event::DragStart &e)
{
}

inline void SqSvgToggleButton::onDragEnd(const event::DragEnd &e)
{
}

inline void SqSvgToggleButton::onDragDrop(const event::DragDrop &e)
{
    if (e.origin != this) {
        return;
    }

    int nextIndex = index + 1;
    if (nextIndex >= (int)frames.size()) {
        nextIndex = 0;
    }

    setIndex(nextIndex);
   
	event::Action eAction;
    if (actionDelegate) {
        actionDelegate->onAction(eAction);
    } else {
	    onAction(eAction);
    }
}


/**
 * A Param widget that wraps a SqSvgToggleButton.
 * We delegate down to the button to do all the button work
 * like drawing and event handling.
 */
class SqSvgParamToggleButton : public ParamWidget
{
public:
    SqSvgParamToggleButton();
    void addFrame(std::shared_ptr<Svg> svg);
    void draw(const DrawArgs &args) override;
    void onAdd(const event::Add&) override;

    void onDragStart(const event::DragStart &e) override;
    void onDragEnd(const event::DragEnd &e) override;
    void onDragDrop(const event::DragDrop &e) override;

    void onAction(const event::Action &e) override;
    void onButton(const event::Button &e) override;
    float getValue();

    // To support toggle manager
    void registerManager(std::shared_ptr<ToggleManager2<SqSvgParamToggleButton>>);
    void turnOff();

    void step() override;
private:


    // the pointer does not imply ownership
    SqSvgToggleButton* button = nullptr;
    std::shared_ptr<ToggleManager2<SqSvgParamToggleButton>> manager;

    bool didStep = false;
    bool isControlKey = false;
};


inline SqSvgParamToggleButton::SqSvgParamToggleButton()
{
    button = new SqSvgToggleButton(this);
    this->addChild(button);
}

inline void SqSvgParamToggleButton::registerManager(std::shared_ptr<ToggleManager2<SqSvgParamToggleButton>> m)
{
    manager = m;
}


inline void SqSvgParamToggleButton::step()
{
    // the first time step is called, we need to get the "offical"
    // param value (set from deserializtion), and propegate that to the button
    if (!didStep) { 
        const float mv = SqHelper::getValue(this);
        float bv = button->getValue();
        if (mv != bv) {
            button->setValue(mv);
        }
    }
    ParamWidget::step();
    this->didStep = true;
}

inline void SqSvgParamToggleButton::turnOff()
{
    button->setValue(0.f);
    SqHelper::setValue(this, 0);
} 

inline void SqSvgParamToggleButton::addFrame(std::shared_ptr<Svg> svg)
{
    button->addFrame(svg);
}

inline void SqSvgParamToggleButton::onAdd(const event::Add&)
{
    button->box.pos = this->box.pos;
    this->box.size = button->box.size;
}

inline float SqSvgParamToggleButton::getValue()
{
    return button->getValue();
}

inline void SqSvgParamToggleButton::draw(const DrawArgs &args)
{
    button->draw(args);
}

void SqSvgParamToggleButton::onButton(const event::Button &e) 
{
    if (e.action == GLFW_RELEASE) {
        isControlKey = e.mods & RACK_MOD_CTRL;
    }

    ParamWidget::onButton(e);
}

inline void SqSvgParamToggleButton::onDragStart(const event::DragStart &e)
{
    button->onDragStart(e);
}

inline void SqSvgParamToggleButton::onDragEnd(const event::DragEnd &e)
{
    button->onDragEnd(e);
}

inline void SqSvgParamToggleButton::onDragDrop(const event::DragDrop &e) 
{
    event::DragDrop e2 = e;
    if (e.origin == this) {
        e2.origin = button;
    }
    button->onDragDrop(e2);

    // normally we tell manager to turn siblings off.
    // control key we don't - allows more than one to be on
    if (!isControlKey) {
        if (manager) {
            manager->go(this);
        }
    }
}

 inline void SqSvgParamToggleButton::onAction(const event::Action &e)
 {
    const float value = getValue();
    SqHelper::setValue(this, value);
 }
