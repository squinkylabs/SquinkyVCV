#pragma once

#include "nanovg.h"
class SqToggleLED : public ModuleLightWidget
{
public:
    SqToggleLED() {
        baseColors.resize(1);
        NVGcolor cx = nvgRGBAf(1, 1, 1, 1);
        baseColors[0] =  cx;
    }


    void addSvg(const char* resourcePath);

#ifdef __V1
   // void onButton(const ButtonEvent &e) override;
    void draw(const DrawArgs &args) override;
#else
   // void onMouseDown(EventMouseDown &e) override;
    void draw(NVGcontext *vg) override;
#endif

private:
    float getValue();
    using SvgPtr = std::shared_ptr<SqHelper::SvgWidget>;
    std::vector<SvgPtr> svgs;
};

inline void SqToggleLED::addSvg(const char* resourcePath)
{
    auto svg = std::make_shared<SqHelper::SvgWidget>();

    SqHelper::setSvg(svg.get(), SqHelper::loadSvg(resourcePath));
    svgs.push_back(svg);
    this->box.size.x = std::max(this->box.size.x, svg->box.size.x);
    this->box.size.y = std::max(this->box.size.y, svg->box.size.y);
}

inline float SqToggleLED::getValue()
{
    return color.a;
}


#ifdef __V1
inline void SqToggleLED::draw(const DrawArgs &args)
{
#else
inline void SqToggleLED::draw(NVGcontext *args)
{
#endif

    const float _value = this->getValue();
    int index = _value > 5 ? 1 : 0;
    auto svg = svgs[index];
    svg->draw(args);
}

