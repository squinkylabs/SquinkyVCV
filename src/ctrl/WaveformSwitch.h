
#pragma once


class WaveCell : public SvgWidget
{
public:
    WaveCell(const char* resOff, const char* resOn);
    void setState(bool);
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

class WaveformSwitch : public rack::ParamWidget
{
public:
    WaveformSwitch();
private:
    FramebufferWidget * fw = nullptr;

    void addSvg(int row, const char* res, const char* resOn);
};

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
    fw->addChild(newCell);
}