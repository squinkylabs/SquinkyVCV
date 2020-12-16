
#pragma once

class Streamer
{
public:
    void setSample(float* data, int frames);
    void setTranspose(bool doTranspoe, float amount);
    bool canPlay();

    // TODO: float 4?
    float step();
public:
    float* data = nullptr;
    int frames = 0;

    int curIntegerSampleOffset = 0;
    bool arePlaying  =false;
    float curFloatSampleOffset = 0;
    bool areTransposing = false;
    bool transposeEnabled = false;
};
