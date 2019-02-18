#pragma once

#include "SqUI.h"
#include "SqHelper.h"

class ToggleButton : public ParamWidget
{
public:
    ToggleButton();
    /**
     * SVG Images must be added in order
     */
    void addSvg(const char* resourcePath);

#ifdef __V1
    void onButton(const event::Button &e) override;
    void draw(const DrawArgs &args) override;
#else
    void onMouseDown(EventMouseDown &e) override;
    void draw(NVGcontext *vg) override;
#endif
    
private:
    using SvgPtr = std::shared_ptr<SVGWidget>;
    std::vector<SvgPtr> svgs;
};

inline ToggleButton::ToggleButton()
{
    this->box.size = Vec(0, 0);
}

inline void ToggleButton::addSvg(const char* resourcePath)
{
    auto svg = std::make_shared<SVGWidget>();
    svg->setSVG(SVG::load(SqHelper::assetPlugin(pluginInstance, resourcePath)));
    svgs.push_back(svg);
    this->box.size.x = std::max(this->box.size.x, svg->box.size.x);
    this->box.size.y = std::max(this->box.size.y, svg->box.size.y);
}

#ifdef __V1
inline void ToggleButton::draw(const DrawArgs &args)
{
    const float _value = SqHelper::getValue(this);
    int index = int(std::round(_value));
    auto svg = svgs[index];
    svg->draw(args);
}
#else
inline void ToggleButton::draw(NVGcontext *vg)
{
    const float _value = SqHelper::getValue(this);
    int index = int(std::round(_value));
    auto svg = svgs[index];
    svg->draw(vg);
}
#endif



#ifdef __V1
inline void ToggleButton::onButton(const event::Button &e)
#else
inline void ToggleButton::onMouseDown(EventMouseDown &e)
#endif
{
    #ifdef __V1
        //only pick the mouse events we care about.
        // TODO: should our buttons be on release, like normal buttons?
        if ((e.button != GLFW_MOUSE_BUTTON_LEFT) ||
            e.action != GLFW_RELEASE) {
                return;
            }
    #endif

    float _value = SqHelper::getValue(this);
    const int index = int(std::round(_value));
    const Vec pos(e.pos.x, e.pos.y);

    if (!svgs[index]->box.contains(pos)) {
        return;
    }

    sq::consumeEvent(&e, this);

    unsigned int v = (unsigned int) std::round(SqHelper::getValue(this));
    if (++v >= svgs.size()) {
        v = 0;

    }

    SqHelper::setValue(this, v);
}


