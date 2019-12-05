
#include "PitchUtils.h"
#include "Scale.h"
#include "ScaleRelativeNote.h"
#include "Triad.h"
#include "asserts.h"

static void test0()
{
    ScalePtr scale = Scale::getScale(Scale::Scales::Major, PitchUtils::c_);
    ScaleRelativeNote root = scale->getScaleRelativeNote(PitchUtils::c_);
    assert(root.valid);
    TriadPtr triad = Triad::make(scale, root, Triad::Inversion::Root);
    triad->assertValid();

    assertEQ(triad->get(0)->degree, 0);
    assertEQ(triad->get(0)->octave, 0);
    assertEQ(triad->get(1)->degree, 2);
    assertEQ(triad->get(1)->octave, 0);
    assertEQ(triad->get(2)->degree, 4);
    assertEQ(triad->get(2)->octave, 0);
}

static void test1()
{
    ScalePtr scale = Scale::getScale(Scale::Scales::Major, PitchUtils::c_);
    ScaleRelativeNote root = scale->getScaleRelativeNote(PitchUtils::c_);
    assert(root.valid);
    TriadPtr triad = Triad::make(scale, root, Triad::Inversion::First);
    triad->assertValid();

    assertEQ(triad->get(0)->degree, 0);
    assertEQ(triad->get(0)->octave, 0);
    assertEQ(triad->get(1)->degree, 2);
    assertEQ(triad->get(1)->octave, -1);
    assertEQ(triad->get(2)->degree, 4);
    assertEQ(triad->get(2)->octave, 0);
}

static void test2()
{
    ScalePtr scale = Scale::getScale(Scale::Scales::Major, PitchUtils::c_);
    ScaleRelativeNote root = scale->getScaleRelativeNote(PitchUtils::c_);
    assert(root.valid);
    TriadPtr triad = Triad::make(scale, root, Triad::Inversion::Second);
    triad->assertValid();

    assertEQ(triad->get(0)->degree, 0);
    assertEQ(triad->get(0)->octave, 0);
    assertEQ(triad->get(1)->degree, 2);
    assertEQ(triad->get(1)->octave, 0);
    assertEQ(triad->get(2)->degree, 4);
    assertEQ(triad->get(2)->octave, -1);
}

void testTriad()
{
    test0();
    test1();
    test2();
}