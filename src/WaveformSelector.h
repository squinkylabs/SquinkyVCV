#pragma once

#include "widgets.hpp"

class ButtonCell
{
public:
    friend class WaveformSelector;
    ButtonCell(const ButtonCell&) = delete;
    ButtonCell& operator = (const ButtonCell&) = delete;

    ButtonCell(float x) : value(x) {}
    void loadSVG(const char* res, const char* resOn);

    const float value;
    rack::Rect box;
private:
    SVGWidget svg;
    SVGWidget svgOn;

};

inline void ButtonCell::loadSVG(const char* res, const char* resOn)
{
    svg.setSVG(SVG::load (assetPlugin(plugin, res)));
    svgOn.setSVG(SVG::load (assetPlugin(plugin, resOn)));
    this->box.size = svg.box.size;
}

using CellPtr = std::shared_ptr<ButtonCell>;

struct WaveformSelector  : OpaqueWidget
{
    WaveformSelector();
    void draw(NVGcontext *vg) override;
    ~WaveformSelector() override;

    std::vector< std::vector< CellPtr>> svgs;
    void addSvg(int row, const char* res, const char* resOn);
    void drawSVG(NVGcontext *vg, SVGWidget&);
    void onMouseDown( EventMouseDown &e ) override;
    CellPtr hitTest(float x, float y);
    //
    float nextValue = 1;

    /**
     * Internal control value.
     * 0 = off
     * 1..n = cell on
     */
    float curValue=0;
};

 CellPtr WaveformSelector::hitTest(float x, float y)
 {
     assert(false);

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
            if (s->box.contains(pos)) {
                return s;
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
   

    // make a new cell, put the SVGs in it
    CellPtr cell = std::make_shared<ButtonCell>(nextValue++);
    cell->loadSVG(res, resOn);
    svgs[row].push_back(cell);

 
    // now set the box for cell
    float y = 0;
    if (row > 0) {
        // if we are going in the second row, y = height of first
        assert(!svgs[row-1].empty());
        CellPtr otherCell = svgs[row-1][0];
        y = otherCell->box.pos.y + otherCell->box.size.y;
    }
    cell->box.pos.y = y;

    if (svgs[row].size() == 1) {
        // If we are the first cell in this row, we are at 0
        cell->box.pos.x = 0;
        printf("just set x to 0 val=%f\n", cell->value);
    } else {
        cell->box.pos.x = 
            svgs[row].back()->box.pos.x +
            svgs[row].back()->box.size.x;
        printf("just set x to %f value=%f\n", cell->box.pos.x, cell->value);
    }
#if 0
    // Now load SVG for the on state
    std::shared_ptr<SVGWidget> p2(new SVGWidget());
    p2->setSVG( SVG::load(assetPlugin(plugin, resOn)));
    cell.svgOn = p2;
    cell.svgOn->box.pos = cell.svg->box.pos; 
#endif

    printf("cell box size=%f, %f po %f, %f\n",
        cell->box.size.x,
        cell->box.size.y,
        cell->box.pos.x,
        cell->box.pos.y); 
}

inline WaveformSelector::WaveformSelector()
{

    addSvg(0, "res/saw_wave.svg", "res/saw_wave_on.svg" );
    addSvg(0, "res/saw_wave.svg", "res/saw_wave_on.svg");

    for (int i=0; i<2; ++i) {
        printf("col %d. %p,%p\n",
            i,
            &svgs[0][i]->svg,
            &svgs[0][2]->svgOn);
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
            const bool on = (curValue == s->value);
            drawSVG(vg, on ? s->svgOn : s->svg);

        }
    }
}

inline void WaveformSelector::onMouseDown( EventMouseDown &e )
{
    e.consumed = false;
    printf("mouse down %f, %f\n", e.pos.x, e.pos.y); fflush(stdout);
 
    CellPtr hit = hitTest(e.pos.x, e.pos.y);
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