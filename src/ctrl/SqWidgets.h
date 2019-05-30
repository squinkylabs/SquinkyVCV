#pragma once

#include "rack.hpp"
#include "WidgetComposite.h"
#include "SqHelper.h"
#include "SqUI.h"
#include <functional>


/**
 * Like Trimpot, but with blue stripe
 */
#if 0
struct BlueTrimmer : SVGKnob
{
    BlueTrimmer()
    {
        minAngle = -0.75*M_PI;
        maxAngle = 0.75*M_PI;
        setSVG(SVG::load(SqHelper::assetPlugin(pluginInstance, "res/BlueTrimmer.svg")));
    }
};
#endif

/**
 * Like Rogan1PSBlue, but smaller.
 */
#ifdef __V1
struct Blue30Knob : SvgKnob
#else
struct Blue30Knob : SVGKnob
#endif
{
    Blue30Knob()
    {
        minAngle = -0.83*M_PI;
        maxAngle = 0.83*M_PI;
       // setSVG(SVG::load(SqHelper::assetPlugin(pluginInstance, "res/Blue30.svg")));
       SqHelper::setSvg(this, SqHelper::loadSvg("res/Blue30.svg"));
    }
};

struct Blue30SnapKnob : Blue30Knob
{
    Blue30SnapKnob()
    {
        snap = true;
        smooth = false;
    }
};

#ifdef __V1
struct NKKSmall : SvgSwitch
#else
struct NKKSmall : SVGSwitch, ToggleSwitch
#endif
{
    NKKSmall()
    {
        addFrame(SqHelper::loadSvg("res/NKKSmall_0.svg"));
        addFrame(SqHelper::loadSvg("res/NKKSmall_1.svg"));
        addFrame(SqHelper::loadSvg("res/NKKSmall_2.svg"));
    }
};

#ifdef __V1
struct BlueToggle : public SvgSwitch
#else 
struct BlueToggle : public SVGSwitch, ToggleSwitch
#endif
{
    BlueToggle()
    {
        addFrame(SqHelper::loadSvg("res/BluePush_1.svg"));
        addFrame(SqHelper::loadSvg("res/BluePush_0.svg"));
    }
};

/**
 * A very basic momentary push button.
 */
#ifdef __V1
struct SQPush : SvgButton
#else
struct SQPush : SVGButton
#endif
{
    SQPush()
    {
#ifdef __V1
        addFrame(SqHelper::loadSvg("res/BluePush_0.svg"));
        addFrame(SqHelper::loadSvg("res/BluePush_1.svg"));
#else
        setSVGs(
            SVG::load(SqHelper::assetPlugin(pluginInstance, "res/BluePush_0.svg")),
            SVG::load(SqHelper::assetPlugin(pluginInstance, "res/BluePush_1.svg"))
        );
#endif
    }

    SQPush(const char* upSVG, const char* dnSVG)
    {
#ifdef __V1
        addFrame(SqHelper::loadSvg(upSVG));
        addFrame(SqHelper::loadSvg(dnSVG));
#else
        setSVGs(
            SVG::load(SqHelper::assetPlugin(pluginInstance, upSVG)),
            SVG::load(SqHelper::assetPlugin(pluginInstance, dnSVG))
        );
#endif
    }
    void center(Vec& pos)
    {
        this->box.pos = pos.minus(this->box.size.div(2));
    }
#ifdef __V1
     void onButton(const event::Button &e) override
     {
        //only pick the mouse events we care about.
        // TODO: should our buttons be on release, like normal buttons?
        // Probably should use drag end
        if ((e.button != GLFW_MOUSE_BUTTON_LEFT) ||
            e.action != GLFW_RELEASE) {
                return;
            }

        if (clickHandler) {
            clickHandler();
        }
       sq::consumeEvent(&e, this);
     }
#else
    void onDragEnd(EventDragEnd &e) override
    {
        SVGButton::onDragEnd(e);
        if (clickHandler) {
            clickHandler();
        }
    }
#endif

    /**
     * User of button passes in a callback lamba here
     */
    void onClick(std::function<void(void)> callback)
    {
        clickHandler = callback;
    }

    std::function<void(void)> clickHandler;
};



/**************************************************************************
 **
 ** SQPanelItem
 ** a menu item made for expaning panels.
 **
 ************************************************************************/

using SQStatusCallback = std::function<bool()>;
using SQActionCAllback = std::function<void()>;

struct SQPanelItem : MenuItem
{

    SQPanelItem(SQStatusCallback, SQActionCAllback);

#ifdef __V1
    void onAction(const event::Action &e) override
#else
    void onAction(EventAction &e) override
#endif
    {
        actionCallback();
    }

    void step() override
    {
        const bool b = statusCallback();
        rightText = CHECKMARK(b);
    }

    SQStatusCallback statusCallback;
    SQActionCAllback actionCallback;
};

inline SQPanelItem::SQPanelItem(SQStatusCallback s, SQActionCAllback a) :
    statusCallback(s),
    actionCallback(a)
{
    text = "Expanded Panel";
}

/**
 * A Widget that holds values and will serialize,
 * But doesn't interacts with mouse.
 */
class NullWidget : public ParamWidget
{
public:

    NullWidget() : ParamWidget()
    {
        // Make sure we have no dimensions to fool 
        // hit testing in VCV.
        box.pos.x = 0;
        box.pos.y = 0;
        box.size.x = 0;
        box.size.y = 0;
    }

    // TODO: port this
#if 0
    // Swallow mouse commands
    void onMouseDown(EventMouseDown &e) override
    {
        e.consumed = false;
    }
#endif

};
