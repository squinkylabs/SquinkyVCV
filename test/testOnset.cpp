
#include "asserts.h"
#include "FFTData.h"

#include <vector>

class OnsetDetector
{
public:
    OnsetDetector(int size) : inputBuffer(size)
    {
    }

    void captureSample(float data)
    {
        if (inputIndex < inputBuffer.size()) {
           // inputBuffer[inputIndex++] = data;
            inputBuffer.set(inputIndex++, data);
        }

        if (inputIndex == inputBuffer.size())  {
            haveData = true;
            inputIndex = 0;
        }

    }

    /**
     * will get the results from last buffer, and clear it.
     * return.first is true if data from last frame was available
     * second it onset or not
     */
    std::pair<bool, bool> wasOnset()
    {
        bool hadData = haveData;
        haveData = false;
        return std::make_pair(hadData, false);
    }
private:
    FFTDataReal inputBuffer;
    int inputIndex = 0;
    bool haveData = false;
};

static void test0()
{
    OnsetDetector o(512);
    assertEQ(o.wasOnset().first, false);
    o.captureSample(0);
    assertEQ(o.wasOnset().first, false);
}

static void test1()
{
    OnsetDetector o(512);
    for (int i = 0; i < 511; ++i) {
        o.captureSample(0);
        assertEQ(o.wasOnset().first, false);
    }
   
    o.captureSample(0);
    assertEQ(o.wasOnset().first, true);
    assertEQ(o.wasOnset().first, false);

   
}

void testOnset()
{
    test0();
    test1();

}