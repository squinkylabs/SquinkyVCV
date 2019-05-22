#pragma once

class PeakDetector
{
public:
    void step(float x)
    {
        value = std::max(value, x);
    }
    float get() const
    {
        return value;
    }
    void decay(float sec)
    {
        value -= 4 * sec;
        value = std::max(value, 0.f);
    }
private:
    float value = 0;
};
