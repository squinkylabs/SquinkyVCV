
#ifndef _MSC_VER 

#include "F2.h"
#include "TestComposite.h"
#include "asserts.h"

template <class T>
inline void testArbitrary( std::function<void(T&)> setup)
{

}

using Comp2 = F2<TestComposite>;

static void testF2Fc()
{
    auto setup = [](Comp2&) {

    };
    testArbitrary<Comp2>(setup);
}

void testFilterComposites()
{
    testF2Fc();
}
#endif