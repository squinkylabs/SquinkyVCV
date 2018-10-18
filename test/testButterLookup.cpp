


#include "BiquadParams.h"
#include "ButterworthFilterDesigner.h"
#include "LookupTable.h"

class ButterworthLookup
{
public:
    ButterworthLookup();
    void get(BiquadParams<float, 2>& params, float normalizedCutoff);
private:
    static const int numTables = 10;
    LookupTableParams<float> tables[numTables];    // five params per two biquads
};

ButterworthLookup::ButterworthLookup()
{
    const int numBins = 256;
    for (int index = 0; index < numTables; ++index) {
        LookupTable<float>::init(tables[index], numBins, 100.0f / 44100.0f, 2000 / 44100.0f, [index](double x) {
               // first design a filter at x hz
            BiquadParams<float, 2> params;
            ButterworthFilterDesigner<float>::designFourPoleHighpass(params, float(x));
            // then save off tap 0;
            return params.getAtIndex(index);
            });
    }
}

void ButterworthLookup::get(BiquadParams<float, 2>& params, float normalizedCutoff)
{
    for (int i = 0; i < numTables; ++i) {
        const int stage = i < numTables / 2;
        float p = LookupTable<float>::lookup( tables[i], normalizedCutoff, true);
        params.setAtIndex(p, i);
    }
}


static void testButterLookup0()
{
    ButterworthLookup b;
    BiquadParams<float, 2> params;
    for (int i = 0; i < 10; ++i) {
        params.setAtIndex(0, i);
    }
    b.get(params, .1f);
    for (int i = 0; i < 10; ++i) {
        assert(params.getAtIndex(i) != 0);
    }
}

void  testButterLookup()
{
    printf("butter look disabled\n");
    testButterLookup0();
}