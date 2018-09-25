#pragma once

#include "widgets.hpp"

struct WaveformSelector  : OpaqueWidget
{
    struct Cell
    {
        Cell(float x) : value(x) {}
        std::shared_ptr<SVGWidget> svg;
        const float value;
    };

    WaveformSelector();
    void draw(NVGcontext *vg) override;
    ~WaveformSelector() override;

    std::vector< std::vector< Cell>> svgs;
    void addSvg(int row, const char* res);
    void drawSVG(NVGcontext *vg, SVGWidget&);
    void onMouseDown( EventMouseDown &e ) override;
    Cell* hitTest(float x, float y);
    //
    float nextValue = 1;
    float curValue=0;
};

 WaveformSelector::Cell* WaveformSelector::hitTest(float x, float y)
 {
    const Vec pos(x, y);
    for (auto& r : svgs) {
        for (auto& s : r) {
            //drawSVG(vg, s.svg);
            printf("  v=%f svg = x-%f, y-%f, w-%f, h-%f\n",
                s.value,
                s.svg->box.pos.x,
                s.svg->box.pos.y,
                s.svg->box.size.x,
                s.svg->box.size.y);
            if (s.svg->box.contains(pos)) {
                return &s;
            }
        }
    }
     //return svg.box.contains();
     return nullptr;
 }

inline void WaveformSelector::addSvg(int row, const char* res)
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
   // cell.svg.setSVG( SVG::load(assetPlugin(plugin, res)));
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
}

inline WaveformSelector::WaveformSelector()
{
    addSvg(0, "res/saw_wave.svg");
    addSvg(0, "res/saw_wave.svg");
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
            drawSVG(vg, *s.svg);
        }
    }
}

inline void WaveformSelector::onMouseDown( EventMouseDown &e )
{
    e.consumed = false;
    printf("mouse down %f, %f\n", e.pos.x, e.pos.y);
    Cell* hit = hitTest(e.pos.x, e.pos.y);
    if (hit) {
        e.consumed = true;
        printf("hit test found cell\n");
        if (hit->value == curValue) {
            printf("value same\n");
            return;
        }
        curValue = hit->value;
    } else {
        printf("hit test failed\n");
    }
}