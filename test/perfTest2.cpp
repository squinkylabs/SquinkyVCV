
#include "MultiLag.h"

#include "MeasureTime.h"

extern double overheadOutOnly;
extern double overheadInOut;

static void testMultiLPF()
{
    //Shaper<TestComposite> gmr;
    MultiLPF<8> lpf;

    lpf.setCutoff(.01f);
    float input[8];


    MeasureTime<float>::run(overheadInOut, "multiplp", [&lpf, &input]() {

        float x = TestBuffers<float>::get();
        for (int i = 0; i < 8; ++i) {
            input[i] = x;
        }
        lpf.step(input);

        return lpf.get(3);
        }, 1);
}

static void testMultiLPFMod()
{
    //Shaper<TestComposite> gmr;
    MultiLPF<8> lpf;

    lpf.setCutoff(.01f);
    float input[8];


    MeasureTime<float>::run(overheadInOut, "multiplp mod", [&lpf, &input]() {

        float x = TestBuffers<float>::get();
        float y = TestBuffers<float>::get();
        y = std::abs(y) + .1;
        while (y > .49) {
            y -= .48;
        }
        assert(y > 0);
        assert(y < .5);

        lpf.setCutoff(y);
        for (int i = 0; i < 8; ++i) {
            input[i] = x;
        }
        lpf.step(input);

        return lpf.get(3);
        }, 1);
}

void perfTest2()
{
    testMultiLPF();
    testMultiLPFMod();
}