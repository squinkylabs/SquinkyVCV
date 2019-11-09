
#include "Scale.h"
#include "ScaleRelativeNote.h"

#include "asserts.h"

static void test0()
{
    auto p = Scale::getScale(Scale::Scales::Major, 0);

    // C is in cmaj
    auto srn = p->getScaleRelativeNote(0);
    assert(srn->isValid());

    // C# isn't
    srn = p->getScaleRelativeNote(1);
    assert(!srn->isValid());
}

void testScale()
{

    test0();
}