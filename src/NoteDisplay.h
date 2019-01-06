
#pragma once
#include "nanovg.h"

/**
 * Experiments:
 * 
 * refactor to remove redundant code
 * draw stroked rectangle
 * figure out how to normalize the coordinates.
 */
struct NoteDisplay : OpaqueWidget
{
   // SequencerModule *module;

    void draw(NVGcontext *vg) override
    {
        // draw some squares for fun
       // nvgScale(vg, 2, 2);

        filledRect(vg, nvgRGBA(0xff, 0x00, 0x00, 0xff), 50, 50, 30, 30); 
        filledRect(vg, nvgRGBA(0x00, 0x00, 0xff, 0xff), 0, 0, 30, 30); 

        nvgStrokeColor(vg,  nvgRGBA(0x00, 0xff, 0x00, 0xff));
        nvgBeginPath(vg);
        nvgRect(vg, 100, 100, 40, 10);
        nvgStroke(vg);
    }

    void filledRect(NVGcontext *vg, NVGcolor color, float x, float y, float w, float h)
    {
        nvgFillColor(vg, color);
        nvgBeginPath(vg);
        nvgRect(vg, x, y, w, h);
        nvgFill(vg);
    }
  

    void onMouseDown(EventMouseDown &e) override
    {
        OpaqueWidget::onMouseDown(e);
        std::cout << "onMouseDown " << e.button << std::flush << std::endl;
    }
    void onMouseMove(EventMouseMove &e) override
    {
       //  std::cout << "onMouseMove " << std::flush << std::endl;
        OpaqueWidget::onMouseMove(e);
    }
    void onFocus(EventFocus &e) override
    {
        std::cout << "onFocus " << std::flush << std::endl;
      // OpaqueWidget::onFocus(e);
        e.consumed = true;
    }
    void onText(EventText &e) override
    {
        std::cout << "onText " << std::flush << std::endl;
        OpaqueWidget::onText(e);
    }

    void onKey(EventKey &e) override
    {
        std::cout << "onKey " << e.key << std::flush << std::endl;
        if (!e.consumed) {
            OpaqueWidget::onKey(e);
        }
    }
    void onMouseEnter(EventMouseEnter &e) override
    {
        std::cout << "nmouseenger " << std::flush << std::endl;
    }
   /** Called when another widget begins responding to `onMouseMove` events */
//	virtual void onMouseLeave(EventMouseLeave &e) {}
    //virtual void onFocus(EventFocus &e) {}
    void onDefocus(EventDefocus &e) override

    {
        std::cout << "defoucs " << std::flush << std::endl;
        e.consumed = true;
    }
};
