
#pragma once


class WaveCell : public SvgWidget
{
public:
    WaveCell(const char* resOff, const char* resOn);
    void setState(bool);

    Vec getSize() const 
    {
        return SvgWidget::box.size;
    }
private:
    std::shared_ptr<Svg> svgOff;
    std::shared_ptr<Svg> svgOn;
    bool state = false;
};

inline WaveCell::WaveCell(const char* resOff, const char* resOn)
{
    svgOff = SqHelper::loadSvg(resOff);
    svgOn = SqHelper::loadSvg(resOn);
    setState(false);
}

inline void WaveCell::setState(bool b)
{
    setSvg(b ? svgOn : svgOff);
    state = b;
}

class WaveformSwitch : public ::rack::ParamWidget
{
public:
    WaveformSwitch();
    void step() override;
    void onButton(const event::Button &e) override;
private:
    FramebufferWidget * fw = nullptr;

    void addSvg(int row, const char* res, const char* resOn);
    WaveCell* getCell(int index);
    int hitTest(float x, float y);

    int _col = 0;
    int _row = 0;
    int currentValue = -1;

    std::vector<WaveCell*> cells;
};


 int WaveformSwitch::hitTest(float x, float y)
 {
    const float cellWidth = cells[0]->box.size.x;
    const float cellHeight = cells[0]->box.size.y;
  //  sqDEBUG("hit test(%.2f, %.2f) r,c= %d,%d  dim=%.2f,%.2f\n", x, y, _row, _col, cellWidth, cellHeight);
    if (x < 0 || y < 0) {
        return -1;
    }

    if ( x > ((_col) * cellWidth)) {
        return -1;
    }

    if (y > ((_row + 1) * cellHeight)) {
        return -1;
    }

    float a = std::floor(x / cellWidth);
    float b = std::floor(y / cellHeight);
    int index =  a + _col * b;    
    return index;
 }

inline void WaveformSwitch::onButton(const event::Button &e)
{
    if (e.action == GLFW_PRESS && (e.button == GLFW_MOUSE_BUTTON_LEFT)) {
        int hit = hitTest(e.pos.x, e.pos.y);

        if (hit >= 0) {
            e.consume(this);
            SqHelper::setValue(this, hit);
        }
    }
}

WaveCell* WaveformSwitch::getCell(int index)
{
    return cells[index];
}

void WaveformSwitch::step()
{
    float  fval = SqHelper::getValue(this);
    int val = int( std::round(fval));
    if (val != currentValue) {
        //DEBUG("step found new value %d old=%d\n", val, currentValue);
        if (currentValue >= 0) {
            auto cell = getCell(currentValue);
            if (cell) {
                cell->setState(false);
            }
        }
        auto cell = getCell(val);
        cell->setState(true);
        currentValue = val;
        fw->dirty = true;
    }
    ParamWidget::step();
}

inline WaveformSwitch::WaveformSwitch()
{
    fw = new FramebufferWidget();
    this->addChild(fw);
    addSvg(0, "res/waveforms-6-08.svg", "res/waveforms-6-07.svg");
    addSvg(0, "res/waveforms-6-06.svg", "res/waveforms-6-05.svg");
    addSvg(0, "res/waveforms-6-02.svg", "res/waveforms-6-01.svg");
    addSvg(1, "res/waveforms-6-04.svg", "res/waveforms-6-03.svg");
    addSvg(1, "res/waveforms-6-12.svg", "res/waveforms-6-11.svg");
    addSvg(1, "res/waveforms-6-10.svg", "res/waveforms-6-09.svg");
}

void WaveformSwitch::addSvg(int row, const char* resOff, const char* resOn)
{
    WaveCell* newCell = new WaveCell(resOff, resOn);
    
    if (row != _row) {
        _row = row;
        _col = 0;
    }

    // assume they are all the same size. The it's easy to calc pos
    Vec size = newCell->getSize();
    newCell->box.pos.x = _col * size.x;
    newCell->box.pos.y = _row * size.y;

    ++_col;

    fw->addChild(newCell);
    cells.push_back(newCell);
}