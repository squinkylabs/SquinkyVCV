
#include "StepRecordInput.h"
#include "TestComposite.h"
#include "asserts.h"

static void test0()
{
    // DrumTrigger<TestComposite>;
    Input cv, gate;
    StepRecordInput<Port> sr(cv, gate);

    RecordInputData buffer;
    sr.step();
    bool b = sr.poll(&buffer);
    assert(!b);
}



static void testOneNote()
{
    // DrumTrigger<TestComposite>;
    Input cv, gate;
    StepRecordInput<Port> sr(cv, gate);

    gate.voltages[0] = 10;
    sr.step();

    RecordInputData buffer;
    bool b = sr.poll(&buffer);
    assert(b);
}

void testStepRecordInput()
{
    test0();
    testOneNote();

}