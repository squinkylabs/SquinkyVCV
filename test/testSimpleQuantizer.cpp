#include "asserts.h"
#include "SimpleQuantizer.h"

static void testSimpleQuanizer0()
{
    SimpleQuantizer q({ SimpleQuantizer::Scales::_12Even }, SimpleQuantizer::Scales::_12Even);
    
    assertEQ(q.quantize(0), 0);
}

void testSimpleQuantizer()
{
    testSimpleQuanizer0();
}