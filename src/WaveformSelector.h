#pragma once

#include "widgets.hpp"

struct WaveformSelector  : OpaqueWidget
{
    WaveformSelector();
    void draw(NVGcontext *vg) override;
    ~WaveformSelector() override;

    SVGWidget theSVG;
};

inline WaveformSelector::WaveformSelector()
{
    printf("in ctor of selector\n");
    theSVG.setSVG( SVG::load(assetPlugin(plugin, "res/saw_wave.svg")));
}

inline WaveformSelector::~WaveformSelector()
{
    printf("in dtor of waveform selector");
}

 void inline WaveformSelector:: draw(NVGcontext *vg)
 {
     #if 0
     auto p = theSVG.box.pos;
     auto s = theSVG.box.size;
     printf("pos x = %f, size x = %f\n", p.x, s.x);
     #endif
     theSVG.draw(vg);
 }