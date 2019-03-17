#pragma once

class OneShot
{
public:
    void step();
    void setSampleTime(float);
    void setDelayMs(float milliseconds);
    bool hasFired() const;
    void set();
private:
    float sampleTime = 0;
    float delayMs = 0;
    float accumulator = 0;
    float deadlineSec = 0;
    bool fired = false;
    void update();
};

inline void OneShot::step()
{
    if (fired) {
        return;
    }
    accumulator += sampleTime;
    if (accumulator >= deadlineSec) {
        fired = true;
    }
}

inline void OneShot::update()
{
    deadlineSec = delayMs / 1000;
    accumulator = 0;
    fired = false;
}

inline void OneShot::setSampleTime(float seconds)
{
    sampleTime = seconds;
   //update();
}

inline void OneShot::setDelayMs(float millis)
{
    delayMs = millis;
    update();
}

inline bool OneShot::hasFired() const
{
    return fired;
}

inline void OneShot::set()
{
    fired = false;
    accumulator = 0;
}

