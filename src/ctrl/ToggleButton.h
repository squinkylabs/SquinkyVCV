#pragma once

#include "SqUI.h"
#include "SqHelper.h"

#include <memory>

#ifndef __V1x
#include "window.hpp"
#endif



#if 1   // just use SvgSwitch

// og->addFrame(SqHelper::loadSvg("res/clean-switch-01.svg"));
class ToggleButton : public SvgSwitch
{
public:
    CircularShadow* shadowToDelete = nullptr;

    ~ToggleButton()
    {
        if (shadowToDelete) {
            delete shadowToDelete;
        }
    }
    
    ToggleButton()
    {
        // The default shadow gives a look we don't want 
        auto shadowToDelete = this->shadow;
        this->fb->removeChild(shadowToDelete);

        // old one had default size 0
        this->box.size.y = 0;
        this->box.size.x = 0;
    }

    // Old switch took relative paths into plugin bundle
    void addSvg(const char* resPath)
    {
        addFrame(SqHelper::loadSvg(resPath));
    }

    // old helper
    int getValue() 
    {
        return  SqHelper::getValue(this);
    }
};

#else

class ToggleButton;

/**
 * Implements radio button from separate buttons.
 * clients hold strong pointers to us.
 * we hold weak ptrs to clients
 */
class ToggleManager : public std::enable_shared_from_this<ToggleManager>
{
public:
    void registerClient(ToggleButton*);
    void go(ToggleButton*);
private:
    std::vector<ToggleButton*> clients;
};


class ToggleButton : public ParamWidget
{
public:
    ToggleButton(); 
    /**
     * SVG Images must be added in order
     */
    void addSvg(const char* resourcePath);

    int getValue() 
    {
        return  SqHelper::getValue(this);
    }

    void onButton(const event::Button &e) override;
    void draw(const DrawArgs &args) override;
    void registerManager(std::shared_ptr<ToggleManager>);
    void turnOff();
    
private:
    using SvgPtr = std::shared_ptr<SqHelper::SvgWidget>;
    std::vector<SvgPtr> svgs;
    std::shared_ptr<ToggleManager> manager;
};

inline void ToggleManager::registerClient(ToggleButton* button)
{
    clients.push_back(button);
    std::shared_ptr<ToggleManager> mgr = shared_from_this(); 
    button->registerManager(mgr);
}

inline void ToggleManager::go(ToggleButton* client)
{
    for (auto cl : clients) {
        ToggleButton* pc = cl;
        if (pc != client) {
            pc->turnOff();
        }
    }
}

inline void ToggleButton::registerManager(std::shared_ptr<ToggleManager> m)
{
    manager = m;
}

inline ToggleButton::ToggleButton()
{
    this->box.size = Vec(0, 0);
}

inline void ToggleButton::addSvg(const char* resourcePath)
{
    auto svg = std::make_shared<SqHelper::SvgWidget>();

   // static void setSvg(SvgKnob* knob, std::shared_ptr<Svg> svg)
    //svg->setSVG(SVG::load(SqHelper::assetPlugin(pluginInstance, resourcePath)));
    SqHelper::setSvg(svg.get(), SqHelper::loadSvg(resourcePath));
    svgs.push_back(svg);
    this->box.size.x = std::max(this->box.size.x, svg->box.size.x);
    this->box.size.y = std::max(this->box.size.y, svg->box.size.y);
}

inline void ToggleButton::draw(const DrawArgs &args)
{
    const float _value = SqHelper::getValue(this);
    int index = int(std::round(_value));
    auto svg = svgs[index];
    svg->draw(args);
}

inline void ToggleButton::turnOff()
{
    SqHelper::setValue(this, 0);
} 

inline void ToggleButton::onButton(const event::Button &e)
{
        //only pick the mouse events we care about.
        // TODO: should our buttons be on release, like normal buttons?
        if ((e.button != GLFW_MOUSE_BUTTON_LEFT) ||
            e.action != GLFW_RELEASE) {
                return;
            }

        const bool ctrl = (e.mods & GLFW_MOD_CONTROL);

    // normally we tell manager to turn siblings off.
    // control key we don't - allows more than one to be on
    if (!ctrl) {
        if (manager) {
            manager->go(this);
        }
    }

    float _value = SqHelper::getValue(this);
    const int index = int(std::round(_value));
    const Vec pos(e.pos.x, e.pos.y);

    //if (!svgs[index]->box.contains(pos)) {
    if (!SqHelper::contains(svgs[index]->box, pos)) {
        return;
    }

    sq::consumeEvent(&e, this);

    unsigned int v = (unsigned int) std::round(SqHelper::getValue(this));
    if (++v >= svgs.size()) {
        v = 0;

    }

    SqHelper::setValue(this, v);
}
#endif


