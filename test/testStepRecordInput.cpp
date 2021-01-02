
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

    gate.channels = 1;
    gate.voltages[0] = 10;
    cv.voltages[0] = 2;
    sr.step();

    RecordInputData buffer;
    bool b = sr.poll(&buffer);
    assert(b);
    assert(buffer.type == RecordInputData::Type::noteOn);
    assert(buffer.pitch == 2);

    b = sr.poll(&buffer);
    assert(!b);
    sr.step();
    b = sr.poll(&buffer);
    assert(!b);

    gate.voltages[0] = 0;
    sr.step();

    b = sr.poll(&buffer);
    assert(b);
    assert(buffer.type == RecordInputData::Type::allNotesOff);
}


void testStepRecordInput()
{
    test0();
    testOneNote();
}