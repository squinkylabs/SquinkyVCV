
#include "PitchUtils.h"
#include "Scale.h"
#include "ScaleRelativeNote.h"

#include "asserts.h"

static void testGetScaleRelativeNote1()
{
    // C Maj octave 0
    auto p = Scale::getScale(Scale::Scales::Major, PitchUtils::c);
    assert(p->getScaleRelativeNote(0)->isValid());  // C
    assert(!p->getScaleRelativeNote(1)->isValid());  // C#
    assert(p->getScaleRelativeNote(2)->isValid());  // D
    assert(!p->getScaleRelativeNote(3)->isValid());  // D#
    assert(p->getScaleRelativeNote(4)->isValid());  // E
    assert(p->getScaleRelativeNote(5)->isValid());  // F
    assert(!p->getScaleRelativeNote(6)->isValid());  // F#
    assert(p->getScaleRelativeNote(7)->isValid());  // G
    assert(!p->getScaleRelativeNote(8)->isValid());  // G#
    assert(p->getScaleRelativeNote(9)->isValid());  // A
    assert(!p->getScaleRelativeNote(10)->isValid());  // A#
    assert(p->getScaleRelativeNote(11)->isValid());  // B
}


static void testGetScaleRelativeNote2()
{
    // G Maj octave 0
    auto p = Scale::getScale(Scale::Scales::Major, PitchUtils::g);
    assert(p->getScaleRelativeNote(PitchUtils::g)->isValid());
    assert(!p->getScaleRelativeNote(PitchUtils::g_)->isValid());
    assert(p->getScaleRelativeNote(PitchUtils::a)->isValid());
    assert(!p->getScaleRelativeNote(PitchUtils::a_)->isValid());
    assert(p->getScaleRelativeNote(PitchUtils::b)->isValid());
    assert(p->getScaleRelativeNote(PitchUtils::c)->isValid());
    assert(!p->getScaleRelativeNote(PitchUtils::c_)->isValid());
    assert(p->getScaleRelativeNote(PitchUtils::d)->isValid());
    assert(!p->getScaleRelativeNote(PitchUtils::d_)->isValid());
    assert(p->getScaleRelativeNote(PitchUtils::e)->isValid());
    assert(!p->getScaleRelativeNote(PitchUtils::f)->isValid());
    assert(p->getScaleRelativeNote(PitchUtils::f_)->isValid());
}

static void testGetScaleRelativeNote3()
{
    auto p = Scale::getScale(Scale::Scales::Major, PitchUtils::c);
    assert(p->getScaleRelativeNote(0)->isValid()); 
    assert(p->getScaleRelativeNote(12)->isValid());
    assert(p->getScaleRelativeNote(24)->isValid());
    assert(p->getScaleRelativeNote(48)->isValid());
}

static void testGetSemitone1()
{
    auto p = Scale::getScale(Scale::Scales::Major, PitchUtils::c);
    assertEQ(p->getSemitone(ScaleRelativeNote(0, 0, p)), 0);
    assertEQ(p->getSemitone(ScaleRelativeNote(1, 0, p)), 2);
    assertEQ(p->getSemitone(ScaleRelativeNote(2, 0, p)), 4);
    assertEQ(p->getSemitone(ScaleRelativeNote(3, 0, p)), 5);
    assertEQ(p->getSemitone(ScaleRelativeNote(4, 0, p)), 7);
    assertEQ(p->getSemitone(ScaleRelativeNote(5, 0, p)), 9);
    assertEQ(p->getSemitone(ScaleRelativeNote(6, 0, p)), 11);
}

void testScale()
{
    testGetScaleRelativeNote1();
    testGetScaleRelativeNote2();
    testGetScaleRelativeNote3();
    testGetSemitone1();
}