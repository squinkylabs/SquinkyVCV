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
#ifdef __V1
   // void onButton(const ButtonEvent &e) override;
    void draw(const DrawArgs &args) override;
#else
   // void onMouseDown(EventMouseDown &e) override;
    void draw(NVGcontext *vg) override;
#endif

private:
    float getValue();
};

inline float SqToggleLED::getValue()
{
    return color.a;
}


#ifdef __V1
inline void SqToggleLED::draw(const DrawArgs &args)
{
#else
inline void SqToggleLED::draw(NVGcontext *vg)
{
#endif
}

