
#pragma once

#ifndef __V1x

#include "ToggleManager2.h"

//#include "app.hpp"
//#include "window.hpp"

// let's back-port the v1 svg button, since that's what our sutf expects
struct SvgButtonV1 : ::rack::OpaqueWidget {
	rack::FramebufferWidget *fb;
	CircularShadow *shadow;
	rack::SVGWidget *sw;
	std::vector<std::shared_ptr<SVG>> frames;

	SvgButtonV1();
	void addFrame(std::shared_ptr<SVG> svg);
#if 0
	void onDragStart(EventDragStart &e) override;
	void onDragEnd(EventDragEnd &e) override;
	void onDragDrop(EventDragDrop &e) override;
#endif
};

inline SvgButtonV1::SvgButtonV1()
{
    fb = new ::rack::FramebufferWidget;
	addChild(fb);

	shadow = new CircularShadow;
	fb->addChild(shadow);
	shadow->box.size = ::rack::Vec();

	sw = new ::rack::SVGWidget;
	fb->addChild(sw);
}

inline void SvgButtonV1::addFrame(std::shared_ptr<SVG> svg)
{
	frames.push_back(svg);
	// If this is our first frame, automatically set SVG and size
	if (!sw->svg) {
		sw->setSVG(svg);
		box.size = sw->box.size;
		fb->box.size = sw->box.size;
		// Move shadow downward by 10%
		shadow->box.size = sw->box.size;
		shadow->box.pos = Vec(0, sw->box.size.y * 0.10);
	}
}



#if 0
inline void SvgButtonV1::onDragStart(EventDragStart &e)
{

}
inline void SvgButtonV1::onDragEnd(EventDragEnd &e)
{

}

inline void SvgButtonV1::onDragDrop(EventDragDrop &e)
{

}
#endif


/**
 * An SvgButton, but in stead of being momentary it toggles between values.
 */
class SqSvgToggleButton : public SvgButtonV1
{
public:
    SqSvgToggleButton (::rack::Widget* = nullptr);

    void onDragStart(EventDragStart &e) override;
	void onDragEnd(EventDragEnd &e) override;
	void onDragDrop(EventDragDrop &e) override;
    float getValue() const;
    void setValue(float);
private:
    int index = 0;
    void setIndex(int i);
    ::rack::Widget* actionDelegate = nullptr;

    //sw->setSvg(frames[0]);
};

inline SqSvgToggleButton::SqSvgToggleButton (::rack::Widget* delegate)
{
    actionDelegate = delegate;
}

inline void SqSvgToggleButton::setIndex(int i)
{
    index = i;
    sw->setSVG(frames[index]);
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

inline void SqSvgToggleButton::onDragStart(EventDragStart &e)
{
}

inline void SqSvgToggleButton::onDragEnd(EventDragEnd &e)
{
}

inline void SqSvgToggleButton::onDragDrop(EventDragDrop &e)
{
    if (e.origin != this) {
        return;
    }

    int nextIndex = index + 1;
    if (nextIndex >= (int)frames.size()) {
        nextIndex = 0;
    }

    setIndex(nextIndex);
   
	EventAction eAction;
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
    void addFrame(std::shared_ptr<SVG> svg);
   // void draw(const DrawArgs &args) override;
    void draw(NVGcontext *vg) override;

		// TODO
   // void onAdd(EventAdd&) override;

    void onDragStart(EventDragStart &e) override;
    void onDragEnd(EventDragEnd &e) override;
    void onDragDrop(EventDragDrop &e) override;

    void onAction(EventAction &e) override;
   // void onButton(const event::Button &e) override;
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
  //  bool isControlKey = false;
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
        button->setValue(mv);

	   	button->box.pos = this->box.pos;
    	this->box.size = button->box.size;
    }
    ParamWidget::step();
    this->didStep = true;
}

inline void SqSvgParamToggleButton::turnOff()
{
    button->setValue(0.f);
    SqHelper::setValue(this, 0);
} 

inline void SqSvgParamToggleButton::addFrame(std::shared_ptr<SVG> svg)
{
    button->addFrame(svg);
}

#if 0 // TODO: on step?
inline void SqSvgParamToggleButton::onAdd(const event::Add&)
{
    button->box.pos = this->box.pos;
    this->box.size = button->box.size;
}
#endif

inline float SqSvgParamToggleButton::getValue()
{
    return button->getValue();
}

inline void SqSvgParamToggleButton::draw(NVGcontext *vg)
{
    button->draw(vg);
}


inline void SqSvgParamToggleButton::onDragStart(EventDragStart &e)
{
    button->onDragStart(e);
}

inline void SqSvgParamToggleButton::onDragEnd(EventDragEnd &e)
{
    button->onDragEnd(e);
}

inline void SqSvgParamToggleButton::onDragDrop(EventDragDrop &e) 
{
    EventDragDrop e2 = e;
    if (e.origin == this) {
        e2.origin = button;
    }
    button->onDragDrop(e2);

    // normally we tell manager to turn siblings off.
    // control key we don't - allows more than one to be on
	const bool isControlKey = ::rack::windowIsModPressed();
    if (!isControlKey) {
        if (manager) {
            manager->go(this);
        }
    }
}

 inline void SqSvgParamToggleButton::onAction(EventAction &e)
 {
    const float value = getValue();
    SqHelper::setValue(this, value);
 }


#endif
