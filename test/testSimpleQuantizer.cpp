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

static float getFreq(float cv) {
    return float(261.63 * std::pow(2.0, cv));
}

static void testSimpleQuantizer12J()
{
    std::vector< SimpleQuantizer::Scales> scales = { SimpleQuantizer::Scales::_12Just };
    SimpleQuantizer* p = new SimpleQuantizer(scales,
        SimpleQuantizer::Scales::_12Just);
    auto q = std::shared_ptr<SimpleQuantizer>(p);

    float x = q->quantize(0 * PitchUtils::semitone);
    assertClose(getFreq(x), 261.63f, .001);

    x = q->quantize(1 * PitchUtils::semitone);
    assertClose(getFreq(x), 279.07f, .01);

    x = q->quantize(2 * PitchUtils::semitone);
    assertClose(getFreq(x), 294.33f, .01);

    x = q->quantize(3 * PitchUtils::semitone);
    assertClose(getFreq(x), 313.96f, .01);

    x = q->quantize(4 * PitchUtils::semitone);
    assertClose(getFreq(x), 327.03f, .01);

    x = q->quantize(5 * PitchUtils::semitone);
    assertClose(getFreq(x), 348.83f, .02);

    x = q->quantize(6 * PitchUtils::semitone);
    assertClose(getFreq(x), 367.92f, .01);

    x = q->quantize(7 * PitchUtils::semitone);
    assertClose(getFreq(x), 392.44f, .01);

    x = q->quantize(8 * PitchUtils::semitone);
    assertClose(getFreq(x), 418.60f, .01);

    x = q->quantize(9 * PitchUtils::semitone);
    assertClose(getFreq(x), 436.05f, .01);

    x = q->quantize(10 * PitchUtils::semitone);
    assertClose(getFreq(x), 470.93f, .01);

    x = q->quantize(11 * PitchUtils::semitone);
    assertClose(getFreq(x), 490.55f, .01);

    x = q->quantize(12 * PitchUtils::semitone);
    assertClose(getFreq(x), 523.25f, .02);
}

static void testSimpleQuantizer8J()
{
    std::vector< SimpleQuantizer::Scales> scales = { SimpleQuantizer::Scales::_8Just };
    SimpleQuantizer* p = new SimpleQuantizer(scales,
        SimpleQuantizer::Scales::_8Just);
    auto q = std::shared_ptr<SimpleQuantizer>(p);

    // C
    float x = q->quantize(0 * PitchUtils::semitone);
    assertClose(getFreq(x), 261.63f, .001);

    // D
    x = q->quantize(2 * PitchUtils::semitone);
    assertClose(getFreq(x), 294.33f, .01);

    // E
    x = q->quantize(4 * PitchUtils::semitone);
    assertClose(getFreq(x), 327.03f, .01);

    // F
    x = q->quantize(5 * PitchUtils::semitone);
    assertClose(getFreq(x), 348.83f, .02);

    // G
    x = q->quantize(7 * PitchUtils::semitone);
    assertClose(getFreq(x), 392.44f, .01);

    // A
    x = q->quantize(9 * PitchUtils::semitone);
    assertClose(getFreq(x), 436.05f, .01);

    // B
    x = q->quantize(11 * PitchUtils::semitone);
    assertClose(getFreq(x), 490.55f, .01);

    // C
    x = q->quantize(12 * PitchUtils::semitone);
    assertClose(getFreq(x), 523.25f, .02);
}

void testSimpleQuantizer()
{
    testSimpleQuantizerOctave();
    testSimpleQuantizer12Even();
    testSimpleQuantizer8Even();
    testSimpleQuantizerOff();
    testSimpleQuantizer8J();
    testSimpleQuantizer12J();
}