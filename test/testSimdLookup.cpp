
#include "LookupTableFactory.h"
#include "asserts.h"

#include "simd.h"


/*
       entries = (T *) malloc((bins + 1) * 2 * sizeof(T));
        numBins_i = bins;
        */
void copyLookupParams(LookupTableParams<float_4>& out, const LookupTableParams<float>& in)
{
    out.alloc(in.numBins_i);
    out.xMin = float_4(in.xMin);
    out.xMax = float_4(in.xMax);
    out.numBins_i = in.numBins_i;

    const int numEntries = 2 * (in.numBins_i + 1);
    for (int i=0; i< numEntries; ++i) {
        float x = in.entries[i];
        out.entries[i] = float_4(x);
    }
}


static void test0()
{
    LookupTableParams<float_4> p4;
    LookupTableParams<float> pf;
    LookupTableFactory<float>::makeAudioTaper(pf);
    copyLookupParams(p4, pf);

    //LookupTable<float_4>::lookup(p, float_4(0));
}

void testSimdLookup()
{
    test0();
}