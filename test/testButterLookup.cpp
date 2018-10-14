


#include "BiquadParams.h"
#include "LookupTable.h"

class ButterworthLookup
{
public:
    ButterworthLookup();
    void get(BiquadParams<float, 4>& params, float normalizedCutoff);
private:
    static const int numTables = 10;
    LookupTableParams<float> tables[numTables];    // five params per two biquads
};

ButterworthLookup::ButterworthLookup()
{

}

void ButterworthLookup::get(BiquadParams<float, 4>& params, float normalizedCutoff)
{
    for (int i = 0; i < numTables; ++i) {
        const int stage = i < numTables / 2;
        float p = LookupTable<float>::lookup( tables[i], normalizedCutoff, true);
       // hmmm - we really need direct access to the taps here.
    }
}


static void testButterLookup0()
{
    ButterworthLookup b;
    BiquadParams<float, 4> params;
    b.get(params, .1f);
}

void  testButterLookup()
{
    printf("butter look disabled\n");
    //testButterLookup0();
}