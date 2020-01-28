
#include "PitchUtils.h"
#include "Scale.h"
#include "ScaleRelativeNote.h"
#include "Triad.h"
#include "asserts.h"

static void testPar1()
{
    {
    //    std::vector<float> first = {1, 2, 3};
    //    std::vector<float> second = {1.1f, 2.1f, 3.1f};

        std::vector<int> first = {3, 6, 9};
        std::vector<int> second = {4, 7, 10};
        assert(Triad::isParallel(first, second));
        assert(Triad::isParallel(second, first));
    }

    {
     //   std::vector<float> first = {1, 2, 3};
     //   std::vector<float> second = {1.1f, 2.1f, 2.9f};
        std::vector<int> first = {3, 6, 9};
        std::vector<int> second = {4, 7, 8};

        assert(!Triad::isParallel(first, second));
        assert(!Triad::isParallel(second, first));
    }
    {
     //   std::vector<float> first = {1, 2, 3};
     //   std::vector<float> second = {1.1f, 1.9f, 3.1f};

        std::vector<int> first = {3, 6, 9};
        std::vector<int> second = {4, 5, 10};


        assert(!Triad::isParallel(first, second));
        assert(!Triad::isParallel(second, first));
    }
    {
     //   std::vector<float> first = {1, 2, 3};
    //    std::vector<float> second = {.9f, 2.1f, 3.1f};

        std::vector<int> first = {3, 6, 9};
        std::vector<int> second = {2, 7, 10};
        assert(!Triad::isParallel(first, second));
        assert(!Triad::isParallel(second, first));
    }
}


static void testPar2()
{
    printf("testPar2\n");
    {
       // std::vector<float> first = {1.1f, 2.1f, 3.1f};
     //   std::vector<float> second = {1.1f, 2.1f, 3.1f};

        std::vector<int> first = {3, 6, 9};
        std::vector<int> second = {3, 6, 9};
        assert(!Triad::isParallel(first, second));
        assert(!Triad::isParallel(second, first));
    }
#if 0
    {
        std::vector<float> first = {1.1f, 2.1f, 3.1f + PitchUtils::semitone / 100};
        std::vector<float> second = {1.1f, 2.1f, 3.1f};
        assert(!Triad::isParallel(first, second));
        assert(!Triad::isParallel(second, first));
    }
#endif
}

static void testMakeRootPos()
{
    ScalePtr scale = Scale::getScale(Scale::Scales::Major, PitchUtils::c_);
    ScaleRelativeNote root = scale->getScaleRelativeNote(PitchUtils::c_);
    assert(root.valid);
    TriadPtr triad = Triad::make(scale, root, Triad::Inversion::Root);
    triad->assertValid(scale);

    assertEQ(triad->get(0)->degree, 0);
    assertEQ(triad->get(0)->octave, 0);
    assertEQ(triad->get(1)->degree, 2);
    assertEQ(triad->get(1)->octave, 0);
    assertEQ(triad->get(2)->degree, 4);
    assertEQ(triad->get(2)->octave, 0);
}

static void testMakeFirstPos()
{
    ScalePtr scale = Scale::getScale(Scale::Scales::Major, PitchUtils::c_);
    ScaleRelativeNote root = scale->getScaleRelativeNote(PitchUtils::c_);
    assert(root.valid);
    TriadPtr triad = Triad::make(scale, root, Triad::Inversion::First);
    triad->assertValid(scale);

    // in first inversion the root goes up, so we expect.
    // third, fifth, root+1
    assertEQ(triad->get(0)->degree, 2);
    assertEQ(triad->get(0)->octave, 0);
    assertEQ(triad->get(1)->degree, 4);
    assertEQ(triad->get(1)->octave, 0);
  
    assertEQ(triad->get(2)->degree, 0);
    assertEQ(triad->get(2)->octave, 1);
}

static void testMakeSecondPos()
{
    ScalePtr scale = Scale::getScale(Scale::Scales::Major, PitchUtils::c_);
    ScaleRelativeNote root = scale->getScaleRelativeNote(PitchUtils::c_);
    assert(root.valid);
    TriadPtr triad = Triad::make(scale, root, Triad::Inversion::Second);
    triad->assertValid(scale);

    // second raise 1 and 3, so expect
    // fifth, root+1, third+1

    assertEQ(triad->get(0)->degree, 4);
    assertEQ(triad->get(0)->octave, 0);
    assertEQ(triad->get(1)->degree, 0);
    assertEQ(triad->get(1)->octave, 1);
    assertEQ(triad->get(2)->degree, 2);
    assertEQ(triad->get(2)->octave, 1);
}

static void test3()
{
    ScalePtr scale = Scale::getScale(Scale::Scales::Major, PitchUtils::c);
    ScaleRelativeNote root = scale->getScaleRelativeNote(PitchUtils::c);
    assert(root.valid);
    TriadPtr triad = Triad::make(scale, root, Triad::Inversion::Root);

    auto cvs = triad->toCv(scale);
    assertEQ(cvs.size(), 3);
    triad->assertValid(scale);

    auto expected = PitchUtils::semitoneToCV(PitchUtils::c);
    assertEQ(cvs[0], expected);
    expected = PitchUtils::semitoneToCV(PitchUtils::e);
    assertEQ(cvs[1], expected);
    expected = PitchUtils::semitoneToCV(PitchUtils::g);
    assertEQ(cvs[2], expected);
}


static void test4()
{
    ScalePtr scale = Scale::getScale(Scale::Scales::Major, PitchUtils::c);
    ScaleRelativeNote root = scale->getScaleRelativeNote(PitchUtils::c);
    assert(root.valid);
    TriadPtr firstTriad = Triad::make(scale, root, Triad::Inversion::Root);

    // our normal finder (no octave switch) does parallel block chords.
    ScaleRelativeNote nextRoot = scale->getScaleRelativeNote(PitchUtils::d);
    TriadPtr secondTriad = Triad::make(scale, nextRoot, *firstTriad, false);

    assert(secondTriad);
    secondTriad->assertValid(scale);

    auto cvs = secondTriad->toCv(scale);
    assertEQ(cvs.size(), 3);

    // we expect to get second inversion short c->d. Both inversions avoid parallel.
    auto expected0 = PitchUtils::semitoneToCV(PitchUtils::d);
    assertEQ(cvs[0], expected0);
    auto expected1 = PitchUtils::semitoneToCV(PitchUtils::f);
    assertEQ(cvs[1], expected1);
    auto expected2 = PitchUtils::semitoneToCV(PitchUtils::a) - 1;
    assertEQ(cvs[2], expected2);
}

static void test4b()
{
    ScalePtr scale = Scale::getScale(Scale::Scales::Major, PitchUtils::c);
    ScaleRelativeNote root = scale->getScaleRelativeNote(PitchUtils::d);
    assert(root.valid);
    TriadPtr firstTriad = Triad::make(scale, root, Triad::Inversion::Root);

    // our normal finder (no octave switch) does parallel clock chords.
    ScaleRelativeNote nextRoot = scale->getScaleRelativeNote(PitchUtils::c);
    TriadPtr secondTriad = Triad::make(scale, nextRoot, *firstTriad, false);

    assert(secondTriad);
    secondTriad->assertValid(scale);

    auto cvs = secondTriad->toCv(scale);
    assertEQ(cvs.size(), 3);

    auto expected0 = PitchUtils::semitoneToCV(PitchUtils::c);
    assertEQ(cvs[0], expected0);
    auto expected1 = PitchUtils::semitoneToCV(PitchUtils::e);
    assertEQ(cvs[1], expected1);
    auto expected2 = PitchUtils::semitoneToCV(PitchUtils::g);
    assertEQ(cvs[2], expected2);
}

static void test5()
{
    ScalePtr scale = Scale::getScale(Scale::Scales::Major, PitchUtils::c);
    ScaleRelativeNote root = scale->getScaleRelativeNote(PitchUtils::d);
    assert(root.valid);
    TriadPtr firstTriad = Triad::make(scale, root, Triad::Inversion::Root);

    // our normal finder (no octave switch) does parallel clock chords.
    ScaleRelativeNote nextRoot = scale->getScaleRelativeNote(PitchUtils::c);
    TriadPtr secondTriad = Triad::make(scale, nextRoot, *firstTriad, true);

    assert(secondTriad);
    secondTriad->assertValid(scale);

    auto cvs = secondTriad->toCv(scale);
    assertEQ(cvs.size(), 3);

    auto expected0 = PitchUtils::semitoneToCV(PitchUtils::c) + 1;
    assertEQ(cvs[0], expected0);
    auto expected1 = PitchUtils::semitoneToCV(PitchUtils::e);
    assertEQ(cvs[1], expected1);
    auto expected2 = PitchUtils::semitoneToCV(PitchUtils::g);
    assertEQ(cvs[2], expected2);
}



static void testCtoF()
{
    printf("\n***** testCtoF\n");
    // c -> f (Inv II) should be good
    ScalePtr scale = Scale::getScale(Scale::Scales::Major, PitchUtils::c);

    // Root pos C
    ScaleRelativeNote rootC = scale->getScaleRelativeNote(PitchUtils::c);
    assert(rootC.valid);
    TriadPtr cTriad = Triad::make(scale, rootC, Triad::Inversion::Root);


    // I think it should connect to second inv F, down an octave
    ScaleRelativeNote rootF = scale->getScaleRelativeNote(PitchUtils::f - 12);
    assert(rootF.valid);
    TriadPtr fTriad = Triad::make(scale, rootF, Triad::Inversion::Second);

    cTriad->_dump("c first", scale);
    fTriad->_dump("f first my way", scale);

    // this is what I get now 
    TriadPtr fTriad2;
    {
        fTriad2 = Triad::make(scale, rootF, Triad::Inversion::First);
        std::vector<ScaleRelativeNotePtr>& notes = fTriad2->_getNotes();

        notes[0] = ScaleRelativeNote::_testMakeFromDegreeAndOctave2(3, -1);
        notes[1] = ScaleRelativeNote::_testMakeFromDegreeAndOctave2(5, 0);
        notes[2] = ScaleRelativeNote::_testMakeFromDegreeAndOctave2(0, 1);
        fTriad2->_dump("other", scale);
    }

   // static float ratePair(ScalePtr scale, const Triad& first, const Triad& second);
    float x = Triad::ratePair(scale, *cTriad, *fTriad);
    float y = Triad::ratePair(scale, *cTriad, *fTriad2);
    printf("mine rates as %f, the bad one as %f\n", x, y);



    assert(x < y);
}

static void testCtoF2()
{
    printf("\n***** testCtoF2\n");

    ScalePtr scale = Scale::getScale(Scale::Scales::Major, PitchUtils::c);
    ScaleRelativeNote rootC = scale->getScaleRelativeNote(PitchUtils::c);
    assert(rootC.valid);
    TriadPtr cTriad = Triad::make(scale, rootC, Triad::Inversion::Root);

    ScaleRelativeNote f = scale->getScaleRelativeNote(PitchUtils::f);
    //static TriadPtr make(ScalePtr scale, const ScaleRelativeNote& root, const Triad& previousTriad, bool switchOctaves);

    printf("\nwill try to make F chord to follow C\n");
    TriadPtr second =  Triad::make(scale, f, *cTriad, true);

    cTriad->_dump("root c", scale);
    second->_dump("to F", scale);


    auto finalSemis = second->toSemi(scale);
    assertEQ(finalSemis[0], 0);
    assertEQ(finalSemis[1], 5);
    assertEQ(finalSemis[2], 9);
}

void testTriad()
{
    testPar1();
    testPar2();
    testMakeRootPos();
    testMakeFirstPos();
    testMakeSecondPos();
    test3();
    printf("add back missing triad tests\n");
    //test4();
    test4b();
    // test5();
    testCtoF();
    testCtoF2();
}