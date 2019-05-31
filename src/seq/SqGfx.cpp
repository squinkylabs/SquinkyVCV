
#include "SqGfx.h"
#include "UIPrefs.h"

#include "nanovg.h"
#ifdef __V1x
    #include "app.hpp"
#endif

#include "window.hpp"


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
#ifdef __V1x
    int f = rack::APP->window->uiFont->handle;
#else
    int f = rack::gGuiFont->handle;
#endif
    nvgFillColor(vg, UIPrefs::NOTE_COLOR);
    nvgFontFaceId(vg, f);
    nvgFontSize(vg, 14);
    nvgText(vg, x, y,text, nullptr);
}
