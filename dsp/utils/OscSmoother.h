#pragma once


// can this move to SqMath?
#ifndef _CLAMP
#define _CLAMP
namespace std {
    inline float clamp(float v, float lo, float hi)
    {
        assert(lo < hi);
        return std::min(hi, std::max(v, lo));
    }
}
#endif

// super simple sine vco
class Svco2
{
public:
    void setPitch(float pitch) {
        delta = std::clamp(pitch, 0.f, .5f);
    }
    float process() {
        acc += delta;
        if (acc > 1) {
            acc -= 1;
        }
        return (float) std::sin( acc * AudioMath::Pi * 2);
    }
    float _getFrequency() const { return delta;  }
private:
    float acc = 0;
    float delta = 0;
};

class OscSmoother
{
public:
    float step(float input);
    bool isLocked() const;
private:
    int cycleInCurrentGroup = 0;
    bool locked = false;
    Svco2 vco;
}