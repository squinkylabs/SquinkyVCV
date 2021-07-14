
#include "SqGfx.h"

#include "UIPrefs.h"
#include "rack.hpp"

void SqGfx::strokedRect(NVGcontext *vg, NVGcolor color, float x, float y, float w, float h) {
    nvgStrokeColor(vg, color);
    nvgBeginPath(vg);
    nvgRect(vg, x, y, w, h);
    nvgStroke(vg);
}

void SqGfx::filledRect(NVGcontext *vg, NVGcolor color, float x, float y, float w, float h) {
    nvgFillColor(vg, color);
    nvgBeginPath(vg);
    nvgRect(vg, x, y, w, h);
    nvgFill(vg);
}

void SqGfx::hBorder(NVGcontext *vg, float thickness, NVGcolor color, float x, float y, float w, float h) {
    nvgFillColor(vg, color);
    nvgBeginPath(vg);
    {
        // bottom edge
        nvgRect(vg, 0, h - thickness, w, thickness);

        // top edge
        nvgRect(vg, 0, 0, w, thickness);
    }
    nvgFill(vg);
}

void SqGfx::vBorder(NVGcontext *vg, float thickness, NVGcolor color, float x, float y, float w, float h) {
    nvgFillColor(vg, color);
    nvgBeginPath(vg);
    {
        // left edge
        nvgRect(vg, 0, 0, thickness, h);

        // right edge
        nvgRect(vg, w - thickness, 0, thickness, h);
    }
    nvgFill(vg);
}

void SqGfx::border(NVGcontext *vg, float thickness, NVGcolor color, float x, float y, float w, float h) {
    nvgFillColor(vg, color);
    nvgBeginPath(vg);
    {
        // left edge
        nvgRect(vg, 0, 0, thickness, h);

        // bottom edge
        nvgRect(vg, 0, h - thickness, w, thickness);

        // right edge
        nvgRect(vg, w - thickness, 0, thickness, h);

        // top edge
        nvgRect(vg, 0, 0, w, thickness);
    }
    nvgFill(vg);
};

void SqGfx::drawText(NVGcontext *vg, float x, float y, const char *text, int size) {
    int f = ::rack::appGet()->window->uiFont->handle;

    // It's a hack to hard code color. Change it later.
    nvgFillColor(vg, UIPrefs::DRAG_TEXT_COLOR);
    nvgFontFaceId(vg, f);
    nvgFontSize(vg, 16);
    nvgText(vg, x, y, text, nullptr);
}

void SqGfx::drawText2(NVGcontext *vg, float x, float y, const char *text, int size, NVGcolor color) {
    int f = ::rack::appGet()->window->uiFont->handle;

    nvgFillColor(vg, color);
    nvgFontFaceId(vg, f);
    nvgFontSize(vg, size);
    nvgText(vg, x, y, text, nullptr);
}

void SqGfx::drawTextBox(NVGcontext *vg, float x, float y, float breakWidth, const char *text, int size, NVGcolor color) {
    int f = ::rack::appGet()->window->uiFont->handle;

    nvgFillColor(vg, color);
    nvgFontFaceId(vg, f);
    nvgFontSize(vg, size);
    nvgTextBox(vg, x, y, breakWidth, text, nullptr);
}