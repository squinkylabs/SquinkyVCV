#pragma once

#include "widgets.hpp"

struct WaveformSelector  : OpaqueWidget
{
    WaveformSelector();
    void draw(NVGcontext *vg) override;
    ~WaveformSelector() override;

   // SVGWidget theSVG;
   std::vector< std::vector< SVGWidget>> svgs;
   void addSvg(int row, const char* res);
   void drawSVG(NVGcontext *vg, SVGWidget&);
};

inline void WaveformSelector::addSvg(int row, const char* res)
{
    if ((int)svgs.size() < row+1) {
        svgs.resize(row+1);
    }

   // std::shared_ptr<SVG> w = SVG::load(assetPlugin(plugin, res));
    SVGWidget w;
    w.setSVG( SVG::load(assetPlugin(plugin, res)));
    svgs[row].push_back(w);
    
    float y = 0;
    if (row > 0) {
        assert(!svgs[row-1].empty());
        const SVGWidget& svg = svgs[row-1][0];
        y = svg.box.pos.y + svg.box.size.y;
    }
    if (svgs[row].size() == 1) {
        w.box.pos.x = 0;
        w.box.pos.y = y;
        // still need to find y offset
    }
}

inline WaveformSelector::WaveformSelector()
{
    printf("in ctor of selector\n"); fflush(stdout);
    addSvg(0, "res/saw_wave.svg");
    addSvg(0, "res/saw_wave.svg");

    #if 0
    svgs.resize(2);
    svgs[0].setSVG( SVG::load(assetPlugin(plugin, "res/saw_wave.svg")));
    svgs[1].setSVG( SVG::load(assetPlugin(plugin, "res/saw_wave.svg")));
   
    svgs[1].box.pos.x = svgs[0].box.size.x;

    this->box.size.x = svgs[0].box.size.x * 2;
    this->box.size.y = svgs[0].box.size.y;
    #endif

#if 0
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

inline void WaveformSelector::drawSVG(NVGcontext *vg, SVGWidget& svg)
{
    float transform[6];
    nvgTransformIdentity(transform);
    nvgTransformTranslate(transform, svg.box.size.x, 0);
    nvgTransform(vg, transform[0], transform[1], transform[2], transform[3], transform[4], transform[5]);

    svg.draw(vg);

}
void inline WaveformSelector::draw(NVGcontext *vg)
{
    for (auto r : svgs) {
        for (auto s : r) {
            drawSVG(vg, s);
        }
    }
    #if 0
    svgs[1].draw(vg);
    float transform[6];
    nvgTransformIdentity(transform);
    nvgTransformTranslate(transform, svgs[0].box.size.x, 0);
    nvgTransform(vg, transform[0], transform[1], transform[2], transform[3], transform[4], transform[5]);

    svgs[1].draw(vg);
    #endif
}