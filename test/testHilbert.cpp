
#include "HilbertFilterDesigner.h"
#include "AudioMath.h"
#include "BiquadParams.h"

#include <cmath>
#include <algorithm>
#include <assert.h>

// test that we can hook it up
template<typename T>
void test0()
{
    BiquadParams<T, 3> paramsSin;
    BiquadParams<T, 3> paramsCos;
    HilbertFilterDesigner<T>::design(44100, paramsSin, paramsCos);
}

// test that filter designer does something (more than just generate zero
template<typename T>
void test1()
{
    BiquadParams<T, 3> paramsSin;
    BiquadParams<T, 3> paramsCos;
    HilbertFilterDesigner<T>::design(44100, paramsSin, paramsCos);

    const double delta = .00001;
    for (int i = 0; i<3; ++i) {
        assert(!AudioMath::closeTo(0, (paramsSin.A1(i)), delta));
        assert(!AudioMath::closeTo(0, (paramsSin.A2(i)), delta));
        assert(!AudioMath::closeTo(0, (paramsSin.B0(i)), delta));
        assert(!AudioMath::closeTo(0, (paramsSin.B1(i)), delta));
        assert(!AudioMath::closeTo(0, (paramsSin.B2(i)), delta));

        assert(!AudioMath::closeTo(0, (paramsCos.A1(i)), delta));
        assert(!AudioMath::closeTo(0, (paramsCos.A2(i)), delta));
        assert(!AudioMath::closeTo(0, (paramsCos.B0(i)), delta));
        assert(!AudioMath::closeTo(0, (paramsCos.B1(i)), delta));
        assert(!AudioMath::closeTo(0, (paramsCos.B2(i)), delta));
    }
}

template<typename T>
void test()
{
    test0<T>();
    test1<T>();

}

void testHilbert()
{
    test<double>();
    test<float>();
}