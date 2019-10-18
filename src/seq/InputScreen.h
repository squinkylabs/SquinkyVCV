#pragma once

#include <memory>

#include "Rack.hpp"
//#include "../../include/engine/widget/FramebufferWidget.hpp"
#include <widget/FramebufferWidget.hpp>


class InputControl;
using InputControlPtr = std::shared_ptr<InputControl>;

struct InputScreen // : public FramebufferWidget
{
public:

private:
};

using InputScreenPtr = std::shared_ptr<InputScreen>;

class InputScreenSet
{
public:
    ~InputScreenSet();
    void add(InputScreenPtr);
    void show();
private:
    std::vector<InputScreenPtr> screens;
};

using InputScreenSetPtr = std::shared_ptr<InputScreenSet>;

