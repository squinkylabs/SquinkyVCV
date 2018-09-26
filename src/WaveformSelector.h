#pragma once

#include "widgets.hpp"

struct WaveformSelector  : OpaqueWidget
{
    struct Cell
    {
        Cell(float x) : value(x) {}
        std::shared_ptr<SVGWidget> svg;
        std::shared_ptr<SVGWidget> svgOn;
        const float value;
    };

    WaveformSelector();
    void draw(NVGcontext *vg) override;
    ~WaveformSelector() override;

    std::vector< std::vector< Cell>> svgs;
    void addSvg(int row, const char* res, const char* resOn);
    void drawSVG(NVGcontext *vg, SVGWidget&);
    void onMouseDown( EventMouseDown &e ) override;
    Cell* hitTest(float x, float y);
    //
    float nextValue = 1;

    /**
     * Internal control value.
     * 0 = off
     * 1..n = cell on
     */
    float curValue=0;
};

 WaveformSelector::Cell* WaveformSelector::hitTest(float x, float y)
 {
    const Vec pos(x, y);
    for (auto& r : svgs) {
        for (auto& s : r) {
#if 0
            printf("  v=%f svg = x-%f, y-%f, w-%f, h-%f\n",
                s.value,
                s.svg->box.pos.x,
                s.svg->box.pos.y,
                s.svg->box.size.x,
                s.svg->box.size.y);
                #endif
            if (s.svg->box.contains(pos)) {
                return &s;
            }
        }
    }
     //return svg.box.contains();
     return nullptr;
 }

inline void WaveformSelector::addSvg(int row, const char* res, const char* resOn)
{
    if ((int)svgs.size() < row+1) {
        svgs.resize(row+1);
    }

    // SVGWidget w;
    //  std::create_shared<SVGWidget>;
    std::shared_ptr<SVGWidget> p(new SVGWidget());

    Cell cell(nextValue++);
    p->setSVG( SVG::load(assetPlugin(plugin, res)));
    cell.svg = p;

    svgs[row].push_back(cell);
    
    float y = 0;
    if (row > 0) {
        assert(!svgs[row-1].empty());
        auto svg = svgs[row-1][0].svg;
        y = svg->box.pos.y + svg->box.size.y;
    }
    cell.svg->box.pos.y = y;
    if (svgs[row].size() == 1) {
        cell.svg->box.pos.x = 0;
        printf("just set x to 0 val=%f\n", cell.value);
    } else {
        cell.svg->box.pos.x = 
            svgs[row].back().svg->box.pos.x +
            svgs[row].back().svg->box.size.x;
        printf("just set x to %f value=%f\n", cell.svg->box.pos.x, cell.value);
    }

    // Now load SVG for the on state
    std::shared_ptr<SVGWidget> p2(new SVGWidget());
    p2->setSVG( SVG::load(assetPlugin(plugin, resOn)));
    cell.svgOn = p2;
    cell.svgOn->box.pos = cell.svg->box.pos; 

    printf("svgon box size=%f, %f po %f, %f\n",
        cell.svgOn->box.size.x,
        cell.svgOn->box.size.y,
        cell.svgOn->box.pos.x,
        cell.svgOn->box.pos.y);

    printf("load cell, svg=%p on=%p\n", cell.svg.get(), cell.svgOn.get());
}

inline WaveformSelector::WaveformSelector()
{
    addSvg(0, "res/saw_wave.svg", "res/saw_wave_on.svg" );
    addSvg(0, "res/saw_wave.svg", "res/saw_wave_on.svg");

    for (int i=0; i<2; ++i) {
        printf("col %d. %p,%p\n",
            i,
            svgs[0][i].svg.get(),
            svgs[0][2].svgOn.get());
        fflush(stdout);

    }
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
    for (auto& r : svgs) {
        for (auto& s : r) {
            const bool on = (curValue == s.value);
            if (!s.svg) {
                printf("No svg\n"); fflush(stdout);
                return;
            }
             if (!s.svgOn) {
               // printf("No svg On\n"); fflush(stdout);
                return;
            }
            drawSVG(vg, on ? *s.svgOn : *s.svg);
        }
    }
}

inline void WaveformSelector::onMouseDown( EventMouseDown &e )
{
    e.consumed = false;
    printf("mouse down %f, %f\n", e.pos.x, e.pos.y); fflush(stdout);
    Cell* hit = hitTest(e.pos.x, e.pos.y);
    if (hit) {
        e.consumed = true;
        printf("hit test found cell\n"); fflush(stdout);
        if (hit->value == curValue) {
            printf("value same\n"); fflush(stdout);
            return;
        }
        curValue = hit->value;
        printf("set curValue=%f\n", curValue); fflush(stdout);
    } else {
        printf("hit test failed\n"); fflush(stdout);
    }
}