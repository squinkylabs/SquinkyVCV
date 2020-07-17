#include "asserts.h"
#include <memory>
#include "PitchUtils.h"
#include "SimpleQuantizer.h"


std::shared_ptr<SimpleQuantizer> makeTest(SimpleQuantizer::Scales scale = SimpleQuantizer::Scales::_12Even)
{
    std::vector< SimpleQuantizer::Scales> scales = { SimpleQuantizer::Scales::_12Even };
    SimpleQuantizer* ptr = new SimpleQuantizer(scales, scale);
    return std::shared_ptr<SimpleQuantizer>(ptr);
}

static void testSimpleQuanizerOctave(SimpleQuantizer::Scales scale)
{
    auto q = makeTest();
    q->setScale(scale);
    //SimpleQuantizer q({ SimpleQuantizer::Scales::_12Even }, SimpleQuantizer::Scales::_12Even);
    assertEQ(q->quantize(0), 0);
    assertEQ(q->quantize(1), 1);
    assertEQ(q->quantize(-1), -1);
    assertEQ(q->quantize(10), 10);
}

static void testSimpleQuanizerOctave() 
{
    testSimpleQuanizerOctave(SimpleQuantizer::Scales::_12Even);
    testSimpleQuanizerOctave(SimpleQuantizer::Scales::_8Even);
    testSimpleQuanizerOctave(SimpleQuantizer::Scales::_12Just);
    testSimpleQuanizerOctave(SimpleQuantizer::Scales::_8Just);
}

static void testSimpleQuanizer12Even()
{
    auto q = makeTest();
    for (int i = -12; i <= 12; ++i) {
        float v = PitchUtils::semitone * i;
        assertClose(q->quantize(v), v, .0001);
    }
}

static void testSimpleQuanizer8Even()
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

static  void testSimpleQuanizerOff()
{
    auto q = makeTest(SimpleQuantizer::Scales::_off);

    assertEQ(q->quantize(0), 0);
    assertEQ(q->quantize(.1f), .1f);
    assertEQ(q->quantize(.01f), .01f);
    assertEQ(q->quantize(9.999f), 9.999f);
}


void testSimpleQuantizer()
{
    testSimpleQuanizerOctave();
    testSimpleQuanizer12Even();
    testSimpleQuanizer8Even();
    testSimpleQuanizerOff();

}