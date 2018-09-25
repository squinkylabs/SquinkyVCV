#pragma once

#include "widgets.hpp"

struct WaveformSelector  : OpaqueWidget
{
    WaveformSelector();
    void draw(NVGcontext *vg) override;
    ~WaveformSelector() override;

   // SVGWidget theSVG;
   std::vector< SVGWidget> svgs;
};

inline WaveformSelector::WaveformSelector()
{
    printf("in ctor of selector\n"); fflush(stdout);
    svgs.resize(2);
    svgs[0].setSVG( SVG::load(assetPlugin(plugin, "res/saw_wave.svg")));
    svgs[1].setSVG( SVG::load(assetPlugin(plugin, "res/saw_wave.svg")));
   
    svgs[1].box.pos.x = svgs[0].box.size.x;

    this->box.size.x = svgs[0].box.size.x * 2;
    this->box.size.y = svgs[0].box.size.y;

#if 1
    printf("svg0 xy=%.2f,%.2f w=%.2f\n", 
        svgs[0].box.pos.x,
        svgs[0].box.pos.y,
        svgs[0].box.size.x
        );
     printf("svg1 xy=%.2f,%.2f w=%.2f\n", 
        svgs[1].box.pos.x,
        svgs[1].box.pos.y,
        svgs[1].box.size.x
        );
     printf("this xy=%.2f,%.2f w=%.2f\n", 
        this->box.pos.x,
        this->box.pos.y,
        this->box.size.x
        );
    auto svg = svgs[1].svg;
    auto h = svg->handle;
    printf("svg1 handle %f %f", h->width, h->height);
#endif
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
    // theSVG.draw(vg);

    svgs[1].draw(vg);
    float transform[6];
    nvgTransformIdentity(transform);
    nvgTransformTranslate(transform, svgs[0].box.size.x, 0);
    nvgTransform(vg, transform[0], transform[1], transform[2], transform[3], transform[4], transform[5]);

    svgs[1].draw(vg);
    #if 0
    for (auto svg : this->svgs) {
        svg.draw(vg);
    }
    #endif
 }