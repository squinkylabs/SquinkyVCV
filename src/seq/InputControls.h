#pragma once

#include "../ctrl/PopupMenuParamWidgetv1.h"

class InputControl
{
public:
    virtual float getValue() const = 0;
    virtual ~InputControl()
    {

    }
};

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