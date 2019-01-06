#pragma once

#include "rack.hpp"
#include <functional>

/**
 * This menu item takes generic lambdas,
 * so can be used for anything
 **/
struct SqMenuItem : rack::MenuItem
{
    void onAction(rack::EventAction &e) override
    {
        _onActionFn();
    }

    void step() override
    {
        rightText = CHECKMARK(_isCheckedFn());
    }

    SqMenuItem(std::function<bool()> isCheckedFn,
        std::function<void()> clickFn) :
        _isCheckedFn(isCheckedFn),
        _onActionFn(clickFn)
    {
    }

private:
    std::function<bool()> _isCheckedFn;
    std::function<void()> _onActionFn;
};


struct ManualMenuItem : SqMenuItem
{
    ManualMenuItem(const char* url) : SqMenuItem(
        []() { return false; },
        [url]() { rack::systemOpenBrowser(url); })
    {
        this->text = "Manual";
    }

};


struct  SqMenuItem_BooleanParam : rack::MenuItem
{
    SqMenuItem_BooleanParam(rack::ParamWidget* widget) :
        widget(widget)
    {
    }
    void onAction(rack::EventAction &e) override
    {
        const float newValue = isOn() ? 0 : 1;
        widget->value = newValue;
        rack::EventChange ec;
        widget->onChange(ec);
        e.consumed = true;
    }

    void step() override
    {
        rightText = CHECKMARK(isOn());
    }

private:
    bool isOn()
    {
        bool ret = widget->value > .5f;
        return ret;
    }
    rack::ParamWidget* const widget;
};