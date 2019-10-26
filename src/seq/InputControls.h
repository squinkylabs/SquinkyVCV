#pragma once

#include "UIPrefs.h"
#include "../ctrl/PopupMenuParamWidgetv1.h"

#include <functional>
#include "rack.hpp"

/**
 * A basic button with callback.
 * Does not get input value
 */
class Button2 : public ::rack::ui::Button
{
public:
    void onAction(const ::rack::event::Action& e) override {
        DEBUG("onAction from button");
        if (handler) {
            handler();
        }
    }
    std::function<void()> handler = nullptr;
};


/**
 * Widgets that control values in and InputScreen
 * must implement this interface.
 */
class InputControl
{
public:
    virtual float getValue() const = 0;
    virtual ~InputControl()
    {
    }
};


/**
 * A basic popup menu selector widget
 */
class InputPopupMenuParamWidget : public PopupMenuParamWidget, public InputControl
{
public:
    float getValue() const override
    {
        int index = 0;
        for (auto label : labels) {
            if (this->text == label) {
                return index;
            }
            ++index;
        }
        assert(false);
        return 0;
    }
};

class CheckBox : public ::rack::widget::OpaqueWidget
{
public:
    void draw(const Widget::DrawArgs &args) override
    {
        NVGcontext *const ctx = args.vg;

        nvgShapeAntiAlias(ctx, true);

        nvgBeginPath(ctx);
        nvgMoveTo(ctx, box.pos.x, box.pos.y);
        nvgLineTo(ctx, box.pos.x + box.size.x, box.pos.y);
        nvgLineTo(ctx, box.pos.x + box.size.x, box.pos.y + box.size.y);
        nvgLineTo(ctx, box.pos.x, box.pos.y + box.size.y);
        nvgLineTo(ctx, box.pos.x, box.pos.y);
        
        nvgStrokeColor(ctx, UIPrefs::TIME_LABEL_COLOR);
      //  nvgStrokePaint
        nvgStrokeWidth(ctx, 1);

        nvgStroke(ctx);
     


    }

};