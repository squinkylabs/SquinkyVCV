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
    float getValue() const override;
};

class CheckBox : public ::rack::widget::OpaqueWidget
{
public:
    void onDragStart(const ::rack::event::DragStart& e) override;
	void onDragEnd(const ::rack::event::DragEnd& e) override;
	void onDragDrop(const ::rack::event::DragDrop& e) override;
    void draw(const Widget::DrawArgs &args) override;

    float getValue() const;
private:
    bool value = false;
    void drawBorder(const Widget::DrawArgs &args);
    void drawX(const Widget::DrawArgs &args);
};