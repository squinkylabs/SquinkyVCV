
#include "SqGfx.h"
#include "UIPrefs.h"

#include "nanovg.h"
#ifdef __V1
#include "app.hpp"
#include "window.hpp"
#endif


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
    int f = APP->window->uiFont->handle;
    nvgFillColor(vg, UIPrefs::NOTE_COLOR);
    nvgFontFaceId(vg, f);
    nvgFontSize(vg, 14);
    nvgText(vg, x, y,text, nullptr);
}
