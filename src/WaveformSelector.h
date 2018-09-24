#pragma once

#include "Widgets.hpp"
struct WaveformSelector  : OpaqueWidget
{
    WaveformSelector();
    void draw(NVGcontext *vg) override;
    ~WaveformSelector() override;
    std::shared_ptr<SVG> theSVG;

};

inline WaveformSelector::WaveformSelector()
{
    printf("in ctor of selector\n");
    theSVG = SVG::load(assetPlugin(plugin, "res/saw_wave.svg"));
}

inline WaveformSelector::~WaveformSelector()
{
    printf("in dtor of waveform selector");
}

 void inline WaveformSelector:: draw(NVGcontext *vg)
 {
     if (theSVG) {
         printf("don't know how to draw svg\n");
         auto h = theSVG->handle;
         printf("h = %p\n", h);
     }
     else printf("no SVG do draw\n");
 }