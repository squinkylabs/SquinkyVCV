
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



static void test3()
{
    ScalePtr scale = Scale::getScale(Scale::Scales::Major, PitchUtils::c);
    ScaleRelativeNote root = scale->getScaleRelativeNote(PitchUtils::c);
    assert(root.valid);
    TriadPtr triad = Triad::make(scale, root, Triad::Inversion::Root);

    auto cvs = triad->toCv(scale);
    assertEQ(cvs.size(), 3);
    triad->assertValid();

    auto expected = PitchUtils::semitoneToCV(PitchUtils::c);
    assertEQ(cvs[0], expected);
    expected = PitchUtils::semitoneToCV(PitchUtils::e);
    assertEQ(cvs[1], expected);
    expected = PitchUtils::semitoneToCV(PitchUtils::g);
    assertEQ(cvs[2], expected);
}

void testTriad()
{
    test0();
    test1();
    test2();
    test3();
}