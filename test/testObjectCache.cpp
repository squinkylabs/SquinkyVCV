
#include "asserts.h"
#include "ObjectCache.h"


extern int _numLookupParams;


static void testBipolar()
{

    assertEQ(_numLookupParams, 0);

    auto test = ObjectCache::getBipolarAudioTaper();
    assertEQ(_numLookupParams, 1);
    auto test2 = ObjectCache::getBipolarAudioTaper();
    assertEQ(_numLookupParams, 1);
    test.reset();
    assertEQ(_numLookupParams, 1);

    test2.reset();
    assertEQ(_numLookupParams, 0);

    {
        // simple test that bipolar audio scalers use cached lookups, and they work
        AudioMath::ScaleFun<float> f = AudioMath::makeBipolarAudioScaler(3, 4);
        assertEQ(f(0, -5, 0), 3.);
        assertEQ(_numLookupParams, 1);
    }
    assertEQ(_numLookupParams, 0);
}










void testObjectCache()
{
    testBipolar();
}