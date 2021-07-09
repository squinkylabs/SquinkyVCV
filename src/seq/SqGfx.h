#pragma once

struct NVGcontext;
struct NVGcolor;

class SqGfx
{
public:
    // These drawing utils could go elsewhere
    static void strokedRect(NVGcontext *vg, NVGcolor color, float x, float y, float w, float h);
    static void filledRect(NVGcontext *vg, NVGcolor color, float x, float y, float w, float h);
    static void drawText(NVGcontext *vg, float x, float y, const char* text, int size = 14);
    static void border(NVGcontext *vg, float thickness, NVGcolor color, float x, float y, float w, float h);
    static void hBorder(NVGcontext *vg, float thickness, NVGcolor color, float x, float y, float w, float h);
    static void vBorder(NVGcontext *vg, float thickness, NVGcolor color, float x, float y, float w, float h);
    static void drawText2(NVGcontext *vg, float x, float y, const char* text, int size, NVGcolor color);
};
