#pragma once

namespace rack {
namespace ui {
    struct Menu;
}
namespace app {
    struct ModuleWidget;
}
}

using Menu = ::rack::ui::Menu;
using ModuleWidget = ::rack::app::ModuleWidget;

class ClockFinder
{
public:

    static void go(ModuleWidget* self, int clockDivSetting, int clockInput, int runInput, int resetInput);

};