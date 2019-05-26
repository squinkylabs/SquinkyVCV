
#ifndef __v1

#include "app.hpp"



// let's back-port the v1 svg button, since that's what our sutf expects
struct SvgButtonV1 : rack::OpaqueWidget {
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
    fb = new rack::FramebufferWidget;
	addChild(fb);

	shadow = new CircularShadow;
	fb->addChild(shadow);
	shadow->box.size = rack::Vec();

	sw = new rack::SVGWidget;
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
    SqSvgToggleButton(rack::Widget* = nullptr);

    void onDragStart(EventDragStart &e) override;
	void onDragEnd(EventDragEnd &e) override;
	void onDragDrop(EventDragDrop &e) override;
    float getValue() const;
private:
    int index = 0;
    void setIndex(int i);
    rack::Widget* actionDelegate = nullptr;

    //sw->setSvg(frames[0]);
};

inline SqSvgToggleButton::SqSvgToggleButton(rack::Widget* delegate)
{
    actionDelegate = delegate;
}

inline void SqSvgToggleButton::setIndex(int i)
{
    index = i;
    sw->setSVG(frames[index]);
    fb->dirty = true;
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


#endif
