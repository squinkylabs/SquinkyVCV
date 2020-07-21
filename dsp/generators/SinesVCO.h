#pragma once

template <typename T>
class SinesVCO
{
public:
    /** Units are standard volts.
     * 0 = C4
     */
    void setPitch(T f);
    T process(T deltaT);
private:
    T phase = 0;
    T freq = 0;
};