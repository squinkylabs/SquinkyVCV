#include "asserts.h"
#include <memory>
#include "PitchUtils.h"
#include "SimpleQuantizer.h"

#include "SqLog.h"


std::shared_ptr<SimpleQuantizer> makeTest(SimpleQuantizer::Scales scale = SimpleQuantizer::Scales::_12Even)
{
    std::vector< SimpleQuantizer::Scales> scales = { SimpleQuantizer::Scales::_12Even };
    SimpleQuantizer* ptr = new SimpleQuantizer(scales, scale);
    return std::shared_ptr<SimpleQuantizer>(ptr);
}

static void testSimpleQuantizerOctave(SimpleQuantizer::Scales scale)
{
    auto q = makeTest();
    q->setScale(scale);
    //SimpleQuantizer q({ SimpleQuantizer::Scales::_12Even }, SimpleQuantizer::Scales::_12Even);
    assertEQ(q->quantize(0), 0);
    assertEQ(q->quantize(1), 1);
    assertEQ(q->quantize(-1), -1);
    assertEQ(q->quantize(10), 10);

    // check that we round towards even semis
    assertEQ(q->quantize(0 + PitchUtils::semitone * .4f), 0);
    assertEQ(q->quantize(0 - PitchUtils::semitone * .4f), 0);
}

static void testSimpleQuantizerOctave() 
{
    testSimpleQuantizerOctave(SimpleQuantizer::Scales::_12Even);
    testSimpleQuantizerOctave(SimpleQuantizer::Scales::_8Even);
    testSimpleQuantizerOctave(SimpleQuantizer::Scales::_12Just);
    testSimpleQuantizerOctave(SimpleQuantizer::Scales::_8Just);
}

static void testSimpleQuantizer12Even()
{
    auto q = makeTest();
    for (int i = -12; i <= 12; ++i) {
        float v = PitchUtils::semitone * i;
        assertClose(q->quantize(v), v, .0001);
    }
}

static void testSimpleQuantizer8Even()
{
    std::vector< SimpleQuantizer::Scales> scales = { SimpleQuantizer::Scales::_12Even,  SimpleQuantizer::Scales::_8Even };
    SimpleQuantizer* p = new SimpleQuantizer(scales,
        SimpleQuantizer::Scales::_8Even);
    auto q =  std::shared_ptr<SimpleQuantizer>(p);

    const float s = PitchUtils::semitone;

    assertClose(q->quantize(0), 0, .0001);              // C
    assertClose(q->quantize(2 * s), 2 * s, .0001);      // D
    assertClose(q->quantize(4 * s), 4 * s, .0001);      // E
    assertClose(q->quantize(5 * s), 5 * s, .0001);      // F
    assertClose(q->quantize(7 * s), 7 * s, .0001);      // G
    assertClose(q->quantize(9 * s), 9 * s, .0001);      // A
    assertClose(q->quantize(11 * s), 11 * s, .0001);    // B
    assertClose(q->quantize(12 * s), 12 * s, .0001);    // C

    assertClose(q->quantize(1 * s), 0, .0001);      // C#
    assertClose(q->quantize(3 * s), 2 * s, .0001);      // D#
    assertClose(q->quantize(6 * s), 5 * s, .0001);      // F#
    assertClose(q->quantize(8 * s), 7 * s, .0001);      // C#
    assertClose(q->quantize(10 * s), 9 * s, .0001);      // C#

}

static  void testSimpleQuantizerOff()
{
    auto q = makeTest(SimpleQuantizer::Scales::_off);

    assertEQ(q->quantize(0), 0);
    assertEQ(q->quantize(.1f), .1f);
    assertEQ(q->quantize(.01f), .01f);
    assertEQ(q->quantize(9.999f), 9.999f);
}


static void testSimpleQuantizer12J(float evenPitch)
{
    std::vector< SimpleQuantizer::Scales> scales = { SimpleQuantizer::Scales::_12Just };
    SimpleQuantizer* p = new SimpleQuantizer(scales,
        SimpleQuantizer::Scales::_12Just);
    auto q = std::shared_ptr<SimpleQuantizer>(p);
    assertClose(q->quantize(evenPitch), evenPitch, .0001);
}

static void testSimpleQuantizer12J()
{
    testSimpleQuantizer12J(0);
    testSimpleQuantizer12J(-1 + 16.f/15);
    testSimpleQuantizer12J(-1 + 9.f/8);
    testSimpleQuantizer12J(-1 + 6.f/5);
    testSimpleQuantizer12J(-1 + 5.f/4);
    testSimpleQuantizer12J(-1 + 4.f/3);
    testSimpleQuantizer12J(-1 + 45.f/32);
    testSimpleQuantizer12J(-1 + 3.f/2);
    testSimpleQuantizer12J(-1 + 8.f / 5);
    testSimpleQuantizer12J(-1 + 5.f/3);
    testSimpleQuantizer12J(-1 + 9.f/5);
    testSimpleQuantizer12J(-1 + 15.f/8);
    testSimpleQuantizer12J(1);
}

static void testSimpleQuantizer8J(float evenPitch)
{
    std::vector< SimpleQuantizer::Scales> scales = { SimpleQuantizer::Scales::_8Just };
    SimpleQuantizer* p = new SimpleQuantizer(scales,
        SimpleQuantizer::Scales::_8Just);
    auto q = std::shared_ptr<SimpleQuantizer>(p);
    assertClose(q->quantize(evenPitch), evenPitch, .0001);
}

static void testSimpleQuantizer8J()
{
    testSimpleQuantizer8J(0);
    testSimpleQuantizer8J(-1 + 9.f/8);
    testSimpleQuantizer8J(-1 + 5.f/4);
    testSimpleQuantizer8J(-1 + 4.f/ 3);
    testSimpleQuantizer8J(-1 + 3.f/2);
    testSimpleQuantizer8J(-1 + 5.f/3);
    testSimpleQuantizer8J(-1 + 15.f/8);
    testSimpleQuantizer8J(1);
}

static void testSimpleQuantizer12J_steps() {
    std::vector< SimpleQuantizer::Scales> scales = { SimpleQuantizer::Scales::_12Just };
    SimpleQuantizer* p = new SimpleQuantizer(scales,
        SimpleQuantizer::Scales::_12Just);
    auto q = std::shared_ptr<SimpleQuantizer>(p);

    float margin = .0001f;
    float eq = 0;
    float just = 0;
    SQINFO("compare eq=%d just=%f", eq, just);
    assertClose(q->quantize(eq), just, margin);

    eq = PitchUtils::semitone * 1;
    just = -1 + 16.f/15;
    SQINFO("compare eq=%d just=%f", eq, just);
    assertClose(q->quantize(eq), just, margin);

    eq = PitchUtils::semitone * 2;
    just = -1 + 9.f/8;
    SQINFO("compare eq=%d just=%f", eq, just);
    assertClose(q->quantize(eq), just, margin);

    eq = PitchUtils::semitone * 3;
    just = -1 + 6.f/5;
    SQINFO("compare eq=%d just=%f", eq, just);
    assertClose(q->quantize(eq), just, margin);

    eq = PitchUtils::semitone * 4;
    just = -1 + 5.f / 4;
    SQINFO("compare eq=%d just=%f", eq, just);
    assertClose(q->quantize(eq), just, margin);

    eq = PitchUtils::semitone * 5;
    just = -1 + 4.f / 3;
    SQINFO("compare eq=%d just=%f", eq, just);
    assertClose(q->quantize(eq), just, margin);

    eq = PitchUtils::semitone * 6;
    just = -1 + 45.f / 32;
    SQINFO("compare eq=%d just=%f", eq, just);
    assertClose(q->quantize(eq), just, margin);

    eq = PitchUtils::semitone * 7;
    just = -1 + 3.f / 2;
    SQINFO("compare eq=%d just=%f", eq, just);
    assertClose(q->quantize(eq), just, margin);

    eq = PitchUtils::semitone * 8;
    just = -1 + 8.f / 5;
    SQINFO("compare eq=%d just=%f", eq, just);
    assertClose(q->quantize(eq), just, margin);

    eq = PitchUtils::semitone * 9;
    just = -1 + 5.f / 3;
    SQINFO("compare eq=%d just=%f", eq, just);
    assertClose(q->quantize(eq), just, margin);

    eq = PitchUtils::semitone * 10;
    just = -1 + 9.f / 5;
    SQINFO("compare eq=%d just=%f", eq, just);
    assertClose(q->quantize(eq), just, margin);

    eq = PitchUtils::semitone * 11;
    just = -1 + 15.f / 8;
    SQINFO("compare eq=%d just=%f", eq, just);
    assertClose(q->quantize(eq), just, margin);

    eq = PitchUtils::semitone * 12;
    just = 1;
    SQINFO("compare eq=%d just=%f", eq, just);
    assertClose(q->quantize(eq), just, margin);

    assert(false);
}
/*
 testSimpleQuantizer12J(0);
    testSimpleQuantizer12J(-1 + 16.f/15);
    testSimpleQuantizer12J(-1 + 9.f/8);
    testSimpleQuantizer12J(-1 + 6.f/5); 3
    testSimpleQuantizer12J(-1 + 5.f/4); 4
    testSimpleQuantizer12J(-1 + 4.f/3)   5
    testSimpleQuantizer12J(-1 + 45.f/32); 6 
    testSimpleQuantizer12J(-1 + 3.f/2);   7
    testSimpleQuantizer12J(-1 + 8.f / 5);  8
    testSimpleQuantizer12J(-1 + 5.f/3);  9
    testSimpleQuantizer12J(-1 + 9.f/5);  10
    testSimpleQuantizer12J(-1 + 15.f/8);  11
    testSimpleQuantizer12J(1);      12

    */


#if 1
static void testSimpleQuantizer12J_stepsb() {
    std::vector< SimpleQuantizer::Scales> scales = { SimpleQuantizer::Scales::_12Just };
    SimpleQuantizer* p = new SimpleQuantizer(scales,
        SimpleQuantizer::Scales::_12Just);
    auto q = std::shared_ptr<SimpleQuantizer>(p);

    for (int i = 0; i < 4 * 12; ++i) {
        float vIn = i * PitchUtils::semitone * .25f;
        float vOut = q->quantize(vIn);

        int semi = i / 4;
        int frac =  i % 4;
        SQINFO("semi[%d:%d] gives %f", semi, frac, vOut);
    }

}
#endif

void testSimpleQuantizer()
{
    testSimpleQuantizerOctave();
    testSimpleQuantizer12Even();
    testSimpleQuantizer8Even();
    testSimpleQuantizerOff();
    testSimpleQuantizer8J();
    testSimpleQuantizer12J();

    //testSimpleQuantizer12J_stepsb();
    //testSimpleQuantizer12J_steps();

}