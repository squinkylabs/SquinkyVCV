#include "asserts.h"
#include <memory>
#include "PitchUtils.h"
#include "SimpleQuantizer.h"


std::shared_ptr<SimpleQuantizer> makeTest(SimpleQuantizer::Scales = SimpleQuantizer::Scales::_12Even)
{
    std::vector< SimpleQuantizer::Scales> scales = { SimpleQuantizer::Scales::_12Even };
    SimpleQuantizer* ptr = new SimpleQuantizer(scales, SimpleQuantizer::Scales::_12Even);
    return std::shared_ptr<SimpleQuantizer>(ptr);
}

static void testSimpleQuanizer0()
{
    auto q = makeTest();
    //SimpleQuantizer q({ SimpleQuantizer::Scales::_12Even }, SimpleQuantizer::Scales::_12Even);
    assertEQ(q->quantize(0), 0);
    assertEQ(q->quantize(1), 1);
    assertEQ(q->quantize(-1), -1);
    assertEQ(q->quantize(10), 10);
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

    assertClose(q->quantize(0), 0, .0001);
    assertClose(q->quantize(2 * s), 2 * s, .0001);
    assertClose(q->quantize(1 * s), 0, .0001);      // minor 2dn q down

    
}


void testSimpleQuantizer()
{
    testSimpleQuanizer0();
    testSimpleQuanizer12Even();
    testSimpleQuanizer8Even();
}