
#ifdef __V1
#include "app/SvgButton.hpp"

/**
 * An SvgButton, but in stead of being momentary it toggles between values.
 */
class SqSvgToggleButton : public rack::app::SvgButton
{
public:
    SqSvgToggleButton(rack::widget::Widget* = nullptr);
    void onDragStart(const event::DragStart &e) override;
    void onDragEnd(const event::DragEnd &e) override;
    void onDragDrop(const event::DragDrop &e) override;

    float getValue() const;
private:
    int index = 0;
    void setIndex(int i);
    rack::widget::Widget* actionDelegate = nullptr;

    //sw->setSvg(frames[0]);
};

inline SqSvgToggleButton::SqSvgToggleButton(rack::widget::Widget* delegate)
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
 * We delegate downt to the button to do all the button work
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

    float getValue() const;
private:


    // the pointer does not imply ownership
    SqSvgToggleButton* button = nullptr;
};

inline SqSvgParamToggleButton::SqSvgParamToggleButton()
{
    button = new SqSvgToggleButton(this);
    this->addChild(button);
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

inline float SqSvgParamToggleButton::getValue() const
{
    return button->getValue();
}

inline void SqSvgParamToggleButton::draw(const DrawArgs &args)
{
    button->draw(args);
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
}

 inline void SqSvgParamToggleButton::onAction(const event::Action &e)
 {
    const float value = getValue();
    SqHelper::setValue(this, value);
 }
 #endif