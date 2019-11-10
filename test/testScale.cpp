
#include "PitchUtils.h"
#include "Scale.h"
#include "ScaleRelativeNote.h"

#include "asserts.h"

static void testGetScaleRelativeNote1()
{
    // C Maj octave 0
    auto p = Scale::getScale(Scale::Scales::Major, PitchUtils::c);
    assert(p->getScaleRelativeNote(0)->valid);  // C
    assert(!p->getScaleRelativeNote(1)->valid);  // C#
    assert(p->getScaleRelativeNote(2)->valid);  // D
    assert(!p->getScaleRelativeNote(3)->valid);  // D#
    assert(p->getScaleRelativeNote(4)->valid);  // E
    assert(p->getScaleRelativeNote(5)->valid);  // F
    assert(!p->getScaleRelativeNote(6)->valid);  // F#
    assert(p->getScaleRelativeNote(7)->valid);  // G
    assert(!p->getScaleRelativeNote(8)->valid);  // G#
    assert(p->getScaleRelativeNote(9)->valid);  // A
    assert(!p->getScaleRelativeNote(10)->valid);  // A#
    assert(p->getScaleRelativeNote(11)->valid);  // B
}


static void testGetScaleRelativeNote2()
{
    // G Maj octave 0
    auto p = Scale::getScale(Scale::Scales::Major, PitchUtils::g);
    assert(p->getScaleRelativeNote(PitchUtils::g)->valid);
    assert(!p->getScaleRelativeNote(PitchUtils::g_)->valid);
    assert(p->getScaleRelativeNote(PitchUtils::a)->valid);
    assert(!p->getScaleRelativeNote(PitchUtils::a_)->valid);
    assert(p->getScaleRelativeNote(PitchUtils::b)->valid);

    // I'm pretty sure this C case will be in wrong octave
    assert(p->getScaleRelativeNote(PitchUtils::c)->valid);
    assert(!p->getScaleRelativeNote(PitchUtils::c_)->valid);
    assert(p->getScaleRelativeNote(PitchUtils::d)->valid);
    assert(!p->getScaleRelativeNote(PitchUtils::d_)->valid);
    assert(p->getScaleRelativeNote(PitchUtils::e)->valid);
    assert(!p->getScaleRelativeNote(PitchUtils::f)->valid);
    assert(p->getScaleRelativeNote(PitchUtils::f_)->valid);
}


static void testGetScaleRelativeNote4()
{
    // G Maj octave 0
    auto p = Scale::getScale(Scale::Scales::Major, PitchUtils::g);

    auto srn = p->getScaleRelativeNote(PitchUtils::g);
    assert(srn->valid);
    assertEQ(srn->degree, 0);
    assertEQ(srn->octave, 0);

    srn = p->getScaleRelativeNote(PitchUtils::g_);
    assert(!srn->valid);

    srn = p->getScaleRelativeNote(PitchUtils::a);
    assert(srn->valid);
    assertEQ(srn->degree, 1);
    assertEQ(srn->octave, 0);

    srn = p->getScaleRelativeNote(PitchUtils::a_);
    assert(!srn->valid);

    srn = p->getScaleRelativeNote(PitchUtils::b);
    assert(srn->valid);
    assertEQ(srn->degree, 2);
    assertEQ(srn->octave, 0);

    // wrap around
    srn = p->getScaleRelativeNote(PitchUtils::c);
    assert(srn->valid);
    assertEQ(srn->degree, 3);
    assertEQ(srn->octave, -1);

    srn = p->getScaleRelativeNote(PitchUtils::c_);
    assert(!srn->valid);

    srn = p->getScaleRelativeNote(PitchUtils::d);
    assert(srn->valid);
    assertEQ(srn->degree, 4);
    assertEQ(srn->octave, -1);

    srn = p->getScaleRelativeNote(PitchUtils::d_);
    assert(!srn->valid);

    srn = p->getScaleRelativeNote(PitchUtils::e);
    assert(srn->valid);
    assertEQ(srn->degree, 5);
    assertEQ(srn->octave, -1);

    srn = p->getScaleRelativeNote(PitchUtils::f);
    assert(!srn->valid);

    srn = p->getScaleRelativeNote(PitchUtils::f_);
    assert(srn->valid);
    assertEQ(srn->degree, 6);
    assertEQ(srn->octave, -1);
}


static void testGetScaleRelativeNote3()
{
    auto p = Scale::getScale(Scale::Scales::Major, PitchUtils::c);

    auto srn = p->getScaleRelativeNote(0);
    assert(srn->valid);
    assertEQ(srn->degree, 0);
    assertEQ(srn->octave, 0);

    srn = p->getScaleRelativeNote(12);
    assert(srn->valid);
    assertEQ(srn->degree, 0);
    assertEQ(srn->octave, 1);

    srn = p->getScaleRelativeNote(48);
    assert(srn->valid);
    assertEQ(srn->degree, 0);
    assertEQ(srn->octave, 4);
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

static void testRoundTrip(ScalePtr scale, int semitone)
{
    ScaleRelativeNotePtr srn = scale->getScaleRelativeNote(semitone);
    if (srn->valid) {
        // if the note is in key
        int semi2 = scale->getSemitone(*srn);
        assertEQ(semi2, semitone);
    }
}

static void testRoundTrip(ScalePtr scale)
{
    for (int i=0; i< 100; ++i) {
        testRoundTrip(scale, i);
    }
}

static void testRoundTrip(Scale::Scales scale)
{
    for (int i=0; i<12; ++i) {
        auto scaleP = Scale::getScale(scale, i);
        testRoundTrip(scaleP);
    }
}
static void testRoundTrip()
{
    testRoundTrip(Scale::Scales::Major);
}

static void testRTBugCases()
{
    {
        auto p = Scale::getScale(Scale::Scales::Major, PitchUtils::c);
        {
            ScaleRelativeNotePtr srn = p->getScaleRelativeNote(0);
            int semi = p->getSemitone(*srn);
            assertEQ(semi, 0);
        }

        {
            ScaleRelativeNotePtr srn2 = p->getScaleRelativeNote(12);
            int semi2 = p->getSemitone(*srn2);
            assertEQ(semi2, 12);
        }
    }

    {
        auto p = Scale::getScale(Scale::Scales::Major, PitchUtils::c_);
        ScaleRelativeNotePtr srn = p->getScaleRelativeNote(0);
        int semi = p->getSemitone(*srn);
        assertEQ(semi, 0);
    }

}

static void testGetSemitone2()
{
    auto p = Scale::getScale(Scale::Scales::Major, PitchUtils::g);
    assertEQ(p->getSemitone(ScaleRelativeNote(0, 0, p)), PitchUtils::g + 0);
    assertEQ(p->getSemitone(ScaleRelativeNote(1, 0, p)), PitchUtils::g + 2);
    assertEQ(p->getSemitone(ScaleRelativeNote(2, 0, p)), PitchUtils::g + 4);
    assertEQ(p->getSemitone(ScaleRelativeNote(3, 0, p)), PitchUtils::g + 5);
    assertEQ(p->getSemitone(ScaleRelativeNote(4, 0, p)), PitchUtils::g + 7);
    assertEQ(p->getSemitone(ScaleRelativeNote(5, 0, p)), PitchUtils::g + 9);
    assertEQ(p->getSemitone(ScaleRelativeNote(6, 0, p)), PitchUtils::g + 11);
}

static void testMinor()
{
    // E min octave 0
    auto p = Scale::getScale(Scale::Scales::Minor, PitchUtils::e);
    assert(p->getScaleRelativeNote(4)->valid);  // e
    assert(!p->getScaleRelativeNote(5)->valid);  // f
    assert(p->getScaleRelativeNote(6)->valid);  // f#
    assert(p->getScaleRelativeNote(7)->valid);  // g
    assert(!p->getScaleRelativeNote(8)->valid);  // g#
    assert(p->getScaleRelativeNote(9)->valid);  // a
    assert(!p->getScaleRelativeNote(10)->valid);  // a#
    assert(p->getScaleRelativeNote(11)->valid);  // b
    assert(p->getScaleRelativeNote(12)->valid);  // c
    assert(!p->getScaleRelativeNote(13)->valid);  // c#
    assert(p->getScaleRelativeNote(14)->valid);  // d
    assert(!p->getScaleRelativeNote(15)->valid);  // d#
}


static void testPhrygian()
{
    auto p = Scale::getScale(Scale::Scales::Phrygian, PitchUtils::e);
    assert(p->getScaleRelativeNote(4)->valid);  // e
    assert(p->getScaleRelativeNote(5)->valid);  // f
    assert(!p->getScaleRelativeNote(6)->valid);  // f#
    assert(p->getScaleRelativeNote(7)->valid);  // g
    assert(!p->getScaleRelativeNote(8)->valid);  // g#
    assert(p->getScaleRelativeNote(9)->valid);  // a
    assert(!p->getScaleRelativeNote(10)->valid);  // a#
    assert(p->getScaleRelativeNote(11)->valid);  // b
    assert(p->getScaleRelativeNote(12)->valid);  // c
    assert(!p->getScaleRelativeNote(13)->valid);  // c#
    assert(p->getScaleRelativeNote(14)->valid);  // d
    assert(!p->getScaleRelativeNote(15)->valid);  // d#
}

static void testMixo()
{
    auto p = Scale::getScale(Scale::Scales::Mixolydian, PitchUtils::g);
    assert(p->getScaleRelativeNote(7)->valid);  // g
    assert(!p->getScaleRelativeNote(8)->valid);  // g#
    assert(p->getScaleRelativeNote(9)->valid);  // a
    assert(!p->getScaleRelativeNote(10)->valid);  // a#
    assert(p->getScaleRelativeNote(11)->valid);  // b
    assert(p->getScaleRelativeNote(12)->valid);  // c
    assert(!p->getScaleRelativeNote(13)->valid);  // c#
    assert(p->getScaleRelativeNote(14)->valid);  // d
    assert(!p->getScaleRelativeNote(15)->valid);  // d#
    assert(p->getScaleRelativeNote(16)->valid);  // e
    assert(p->getScaleRelativeNote(17)->valid);  // f
    assert(!p->getScaleRelativeNote(18)->valid);  // f#
}


static void testDorian()
{
    auto p = Scale::getScale(Scale::Scales::Dorian, PitchUtils::d);
    assert(p->getScaleRelativeNote(2)->valid);  // D
    assert(!p->getScaleRelativeNote(3)->valid);  // d#
    assert(p->getScaleRelativeNote(4)->valid);  // e
    assert(p->getScaleRelativeNote(5)->valid);  // f
    assert(!p->getScaleRelativeNote(6)->valid);  // f#
    assert(p->getScaleRelativeNote(7)->valid);  // g
    assert(!p->getScaleRelativeNote(8)->valid);  // g#
    assert(p->getScaleRelativeNote(9)->valid);  // a
    assert(!p->getScaleRelativeNote(10)->valid);  // a#
    assert(p->getScaleRelativeNote(11)->valid);  // b
    assert(p->getScaleRelativeNote(12)->valid);  // c
    assert(!p->getScaleRelativeNote(13)->valid);  // c#
}


static void testLydian()
{
    auto p = Scale::getScale(Scale::Scales::Lydian, PitchUtils::f);
    assert(p->getScaleRelativeNote(5)->valid);  // f
    assert(!p->getScaleRelativeNote(6)->valid);  // f#
    assert(p->getScaleRelativeNote(7)->valid);  // g
    assert(!p->getScaleRelativeNote(8)->valid);  // g#
    assert(p->getScaleRelativeNote(9)->valid);  // a
    assert(!p->getScaleRelativeNote(10)->valid);  // a#
    assert(p->getScaleRelativeNote(11)->valid);  // b
    assert(p->getScaleRelativeNote(12)->valid);  // c
    assert(!p->getScaleRelativeNote(13)->valid);  // c#
    assert(p->getScaleRelativeNote(14)->valid);  // d
    assert(!p->getScaleRelativeNote(15)->valid);  // d#
    assert(p->getScaleRelativeNote(16)->valid);  // e
}


static void testLocrian()
{
    auto p = Scale::getScale(Scale::Scales::Locrian, PitchUtils::b);
    assert(p->getScaleRelativeNote(11)->valid);  // b
    assert(p->getScaleRelativeNote(12)->valid);  // c
    assert(!p->getScaleRelativeNote(13)->valid);  // c#
    assert(p->getScaleRelativeNote(14)->valid);  // d
    assert(!p->getScaleRelativeNote(15)->valid);  // d#
    assert(p->getScaleRelativeNote(16)->valid);  // e
    assert(p->getScaleRelativeNote(17)->valid);  // f
    assert(!p->getScaleRelativeNote(18)->valid);  // f#
    assert(p->getScaleRelativeNote(19)->valid);  // g
    assert(!p->getScaleRelativeNote(20)->valid);  // g#
    assert(p->getScaleRelativeNote(21)->valid);  // a
    assert(!p->getScaleRelativeNote(22)->valid);  // a#
}

void testScale()
{
    testGetScaleRelativeNote1();
    testGetScaleRelativeNote2();
    testGetScaleRelativeNote3();
    testGetScaleRelativeNote4();
    testGetSemitone1();
    testGetSemitone2();

    testRTBugCases();
    testRoundTrip();

    testMinor();
    testPhrygian();
    testMixo();
    testDorian();
    testLydian();
    testLocrian();
}