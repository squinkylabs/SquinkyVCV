
#include "rack.hpp"
#include "SqGfx.h"
#include "UIPrefs.h"

void SqGfx::strokedRect(NVGcontext *vg, NVGcolor color, float x, float y, float w, float h)
{
    nvgStrokeColor(vg, color);
    nvgBeginPath(vg);
    nvgRect(vg, x, y, w, h);
    nvgStroke(vg);
}

void SqGfx::filledRect(NVGcontext *vg, NVGcolor color, float x, float y, float w, float h)
{
    nvgFillColor(vg, color);
    nvgBeginPath(vg);
    nvgRect(vg, x, y, w, h);
    nvgFill(vg);
}

void SqGfx::drawText(NVGcontext *vg, float x, float y, const char* text, int size)
{
    int f = ::rack::appGet()->window->uiFont->handle;

    // It's a hack to hard code color. Change it later.
    nvgFillColor(vg, UIPrefs::DRAG_TEXT_COLOR);
    nvgFontFaceId(vg, f);
    nvgFontSize(vg, 16);
    nvgText(vg, x, y, text, nullptr);
}
