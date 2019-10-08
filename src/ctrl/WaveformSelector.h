#pragma once

#include "SqHelper.h"
#ifdef __V1x
#else
#include "widgets.hpp"
#include <GLFW/glfw3.h>
#endif

class ButtonCell
{
public:
    friend struct WaveformSelector;
    ButtonCell(const ButtonCell&) = delete;
    ButtonCell& operator = (const ButtonCell&) = delete;

    ButtonCell(float x) : value(x)
    {
    }
    void loadSVG(const char* res, const char* resOn);

    const float value;
    ::rack::Rect box;

    void dump(const char*);
private:
    SqHelper::SvgWidget svg;
    SqHelper::SvgWidget svgOn;
};

inline void ButtonCell::loadSVG(const char* res, const char* resOn)
{
    SqHelper::setSvg(&svg, SqHelper::loadSvg(res));
    SqHelper::setSvg(&svgOn, SqHelper::loadSvg(resOn));
    this->box.size = svg.box.size;
}

inline void ButtonCell::dump(const char* label)
{
    printf("cell(%.2f) {%s} box size=%f, %f po %f, %f\n",
        value,
        label,
        box.size.x,
        box.size.y,
        box.pos.x,
        box.pos.y);
}

using CellPtr = std::shared_ptr<ButtonCell>;

struct WaveformSelector : ParamWidget
{
    WaveformSelector();
    ~WaveformSelector() override;

    std::vector< std::vector< CellPtr>> svgs;
    void addSvg(int row, const char* res, const char* resOn);
   
    void draw(const DrawArgs &arg) override;
    void onButton(const event::Button &e) override;
    void drawSVG(const DrawArgs &arg, SqHelper::SvgWidget&, float x, float y);
    CellPtr hitTest(float x, float y);
    //
    float nextValue = 0;
    widget::FramebufferWidget* fb = nullptr;
};

CellPtr WaveformSelector::hitTest(float x, float y)
{
    const Vec pos(x, y);
    for (auto& r : svgs) {
        for (auto& s : r) {
            if (SqHelper::contains(s->box,  pos)) {
                return s;
            }
        }
    }
    return nullptr;
}

inline void WaveformSelector::addSvg(int row, const char* res, const char* resOn)
{
    if ((int) svgs.size() < row + 1) {
        svgs.resize(row + 1);
    }

    // make a new cell, put the SVGs in it
    CellPtr cell = std::make_shared<ButtonCell>(nextValue++);
    cell->loadSVG(res, resOn);
    svgs[row].push_back(cell);

    // now set the box for cell
    float y = 0;
    if (row > 0) {
        // if we are going in the second row, y = height of first
        assert(!svgs[row - 1].empty());
        CellPtr otherCell = svgs[row - 1][0];
        y = otherCell->box.pos.y + otherCell->box.size.y;
    }
    cell->box.pos.y = y;

    const int cellsInRow = (int) svgs[row].size();
    if (cellsInRow == 1) {
        cell->box.pos.x = 0;
    } else {
        cell->box.pos.x =
            svgs[row][cellsInRow - 2]->box.pos.x +
            svgs[row][cellsInRow - 2]->box.size.x;
    }
}

inline WaveformSelector::WaveformSelector()
{
    addSvg(0, "res/waveforms-6-08.svg", "res/waveforms-6-07.svg");
    addSvg(0, "res/waveforms-6-06.svg", "res/waveforms-6-05.svg");
    addSvg(0, "res/waveforms-6-02.svg", "res/waveforms-6-01.svg");
    addSvg(1, "res/waveforms-6-04.svg", "res/waveforms-6-03.svg");
    addSvg(1, "res/waveforms-6-12.svg", "res/waveforms-6-11.svg");
    addSvg(1, "res/waveforms-6-10.svg", "res/waveforms-6-09.svg");
}

inline WaveformSelector::~WaveformSelector()
{
}

#if 1

inline void WaveformSelector::drawSVG(const DrawArgs &arg, SqHelper::SvgWidget& svg, float x, float y)
{
    NVGcontext* vg = arg.vg;
    nvgSave(vg);
    float transform[6];
    nvgTransformIdentity(transform);
    nvgTransformTranslate(transform, x, y);
    nvgTransform(vg, transform[0], transform[1], transform[2], transform[3], transform[4], transform[5]);
    svg.draw(arg);
    nvgRestore(vg);
}

inline void WaveformSelector::draw(const DrawArgs &arg)
{
    for (auto& r : svgs) {
        for (auto& s : r) {
            const bool on = SqHelper::getValue(this) == s->value;
            drawSVG(arg, on ? s->svgOn : s->svg, s->box.pos.x, s->box.pos.y);
        }
    }
}

inline void WaveformSelector::onButton(const event::Button &e)
{
        // for now, use both button presses.
        // eventually get more sophisticated.
    if (e.action == GLFW_PRESS && (e.button == GLFW_MOUSE_BUTTON_LEFT || e.button == GLFW_MOUSE_BUTTON_RIGHT)) {
        CellPtr hit = hitTest(e.pos.x, e.pos.y);
        if (hit) {
            e.consume(this);
            if (hit->value == SqHelper::getValue(this)) {
                return;
            }
            SqHelper::setValue(this, hit->value);
        }
    }
}


#endif
