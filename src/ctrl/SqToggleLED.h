#pragma once

#include "nanovg.h"
class SqToggleLED : public ModuleLightWidget
{
public:
    SqToggleLED() {
        baseColors.resize(1);
        NVGcolor cx = nvgRGBAf(1, 1, 1, 1);
        baseColors[0] =  cx;
    }

    /** 
     * path is relative to our plugin,
     * unless pathIsAbsolute
     */
    void addSvg(const char* resourcePath, bool pathIsAbsolute = false);

    /**
     * pass callback here to handle clicking on LED
     */
    using callback = std::function<void(bool isCtrlKey)>;
    void setHandler(callback);

#ifdef __V1x
    void onButton(const event::Button &e) override;
    void draw(const DrawArgs &args) override;
#else
    void onMouseDown(EventMouseDown &e) override;
    void draw(NVGcontext *vg) override;
#endif

private:
    float getValue();
    using SvgPtr = std::shared_ptr<SqHelper::SvgWidget>;
    std::vector<SvgPtr> svgs;
    callback handler = nullptr;
    int getSvgIndex();
};

inline void SqToggleLED::setHandler(callback h)
{
    handler = h;
}

inline void SqToggleLED::addSvg(const char* resourcePath, bool pathIsAbsolute)
{
    auto svg = std::make_shared<SqHelper::SvgWidget>();
    SqHelper::setSvg(svg.get(), SqHelper::loadSvg(resourcePath, pathIsAbsolute));
    svgs.push_back(svg);
    this->box.size.x = std::max(this->box.size.x, svg->box.size.x);
    this->box.size.y = std::max(this->box.size.y, svg->box.size.y);
}

inline float SqToggleLED::getValue()
{
    return color.a;
}

inline int SqToggleLED::getSvgIndex()
{
    const float _value = this->getValue();
    int index = _value > .5f ? 1 : 0;
    return index;
}

#ifdef __V1x
inline void SqToggleLED::draw(const DrawArgs &args)
{
#else
inline void SqToggleLED::draw(NVGcontext *args)
{
#endif

    int index = getSvgIndex();
    auto svg = svgs[index];
    svg->draw(args);
}


#ifdef __V1x
inline void SqToggleLED::onButton(const event::Button &e)
#else
inline void SqToggleLED::onMouseDown(EventMouseDown &e)
#endif
{
    #ifdef __V1x
        //only pick the mouse events we care about.
        // TODO: should our buttons be on release, like normal buttons?
        if ((e.button != GLFW_MOUSE_BUTTON_LEFT) ||
            e.action != GLFW_RELEASE) {
                return;
        }
        const bool ctrlKey = (e.mods & GLFW_MOD_CONTROL);
    #else
        if (e.button != GLFW_MOUSE_BUTTON_LEFT) {
                return;
        }
        const bool ctrlKey = rack::windowIsModPressed();
    #endif

    int index = getSvgIndex();
    const Vec pos(e.pos.x, e.pos.y);

    //if (!svgs[index]->box.contains(pos)) {
    if (!SqHelper::contains(svgs[index]->box, pos)) {
        return;
    }

    sq::consumeEvent(&e, this);

    if (handler) {
        handler(ctrlKey);
    }
}

