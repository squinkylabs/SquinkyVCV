


#include "BiquadParams.h"

class ButterworthLookup
{
public:
    ButterworthLookup();
    void get(BiquadParams<float, 4>& params, float normalizedCutoff);
};

static void testButterLookup0()
{
    ButterworthLookup b;
    BiquadParams<float, 4> params;
    b.get(params, .1f);
}

void  testButterLookup()
{
    testButterLookup0();
}