
#include "MidiAudition.h"
#include "MidiEvent.h"
#include "MidiSelectionModel.h"
#include "TestAuditionHost.h"
#include "TestHost2.h"

#include "asserts.h"

//************************** these tests are for the selection model
static void testSelectNoteAuditions()
{
    auto a = std::make_shared<TestAuditionHost>();
    MidiSelectionModel s(a);

    assert(a->notes.empty());

    MidiNoteEventPtr n = std::make_shared<MidiNoteEvent>();
    n->pitchCV = 0;
    s.select(n);
    
    assert(!a->notes.empty());
    assertEQ(a->notes.size(), 1);
    assertEQ(a->notes[0], 0);
}

static void testNotNoteNoAudition()
{
    auto a = std::make_shared<TestAuditionHost>();
    MidiSelectionModel s(a);

    assert(a->notes.empty());

    MidiEndEventPtr n = std::make_shared<MidiEndEvent>();
   // n->pitchCV = 0;

    s.select(n);

    assert(a->notes.empty());
}

static void testSelectThreeBothAudition()
{
    auto a = std::make_shared<TestAuditionHost>();
    MidiSelectionModel s(a);

    assert(a->notes.empty());

    MidiNoteEventPtr n1 = std::make_shared<MidiNoteEvent>();
    MidiNoteEventPtr n2 = std::make_shared<MidiNoteEvent>();
    MidiNoteEventPtr n3 = std::make_shared<MidiNoteEvent>();
    n1->pitchCV = 1;
    n2->pitchCV = 2;
    n3->pitchCV = 3;
    s.select(n1);
    s.extendSelection(n2);
    s.addToSelection(n3, false);

    assertEQ(a->notes.size(), 3);
    assertEQ(a->notes[0], 1);
    assertEQ(a->notes[1], 2);
    assertEQ(a->notes[2], 3);
}


static void testSelectNoteTwiceAuditionsOnceSub(bool keepExisting)
{
    auto a = std::make_shared<TestAuditionHost>();
    MidiSelectionModel s(a);

    assert(a->notes.empty());

    MidiNoteEventPtr n = std::make_shared<MidiNoteEvent>();
    n->pitchCV = 0;
    s.select(n);
    assertEQ(s.size(), 1);

    assert(!a->notes.empty());
    assertEQ(a->notes.size(), 1);
    assertEQ(a->notes[0], 0);

    // add second note, but it's the same, do will do nothing
    MidiNoteEventPtr n2 = std::make_shared<MidiNoteEvent>();
    n2->pitchCV = 0;
    s.addToSelection(n2, keepExisting);

    assertEQ(s.size(), 1);

    assertEQ(s.size(), 1);
    assert(!a->notes.empty());
    assertEQ(a->notes.size(), 1);
    assertEQ(a->notes[0], 0);
}

static void testSelectNoteTwiceAuditionsOnce()
{
    testSelectNoteTwiceAuditionsOnceSub(true);
}


static void testSelectNoteTwiceAuditionsOnce2()
{
    testSelectNoteTwiceAuditionsOnceSub(false);
}



//******************These tests are for MidiAudition

static void testPlaysNote()
{
    std::shared_ptr<TestHost2> host = std::make_shared<TestHost2>();
    MidiAudition a(host);
    a.enable(true);

    a.auditionNote(5);

    assertEQ(host->gateChangeCount, 1);
    assert(host->gateState[0]);
    assertEQ(host->cvValue[0], 5);
}

static void testNoteStops()
{
    printf("testNoteStops\n");
    const float sampleRate = 44100;
    const float sampleTime = 1.0f / sampleRate;
    std::shared_ptr<TestHost2> host = std::make_shared<TestHost2>();
    MidiAudition a(host);
    a.setSampleTime(sampleTime);
    a.enable(true);

    float samplesToStop = MidiAudition::noteDurationSeconds() / sampleTime;
    int notEnoughSamples = int(samplesToStop - 10);

    a.auditionNote(5);
    assertEQ(host->gateChangeCount, 1);
    assert(host->gateState[0]);
    assertEQ(host->cvValue[0], 5);

    a.sampleTicksElapsed(notEnoughSamples);
    assertEQ(host->gateChangeCount, 1);
    assert(host->gateState[0]);

    a.sampleTicksElapsed(20);
    assertEQ(host->gateChangeCount, 2);
    assert(!host->gateState[0]);
}

static void testNoteRetrigger()
{
    const float sampleRate = 44100;
    const float sampleTime = 1.0f / sampleRate;
    std::shared_ptr<TestHost2> host = std::make_shared<TestHost2>();
    MidiAudition a(host);
    a.setSampleTime(sampleTime);
    a.enable(true);

    float samplesToStop = MidiAudition::noteDurationSeconds() / sampleTime;
    int notEnoughSamples = int(samplesToStop / 2.f);

    a.auditionNote(5);
    assertEQ(host->gateChangeCount, 1);
    assert(host->gateState[0]);
    assertEQ(host->cvValue[0], 5);

    a.sampleTicksElapsed(notEnoughSamples);
    assertEQ(host->gateChangeCount, 1);
    assert(host->gateState[0]);

    // audition note again, should re-trigger
    a.auditionNote(6);
    assertEQ(host->gateChangeCount, 2);
    assert(!host->gateState[0]);
    assertEQ(host->cvValue[0], 5);

    // gate should still be held in re-trigger.
    float retriggerSamples = MidiAudition::retriggerDurationSeconds() / sampleTime;
    notEnoughSamples = int(retriggerSamples - 10);
    a.sampleTicksElapsed(notEnoughSamples);
    assertEQ(host->gateChangeCount, 2);
    assert(!host->gateState[0]);
    assertEQ(host->cvValue[0], 5);

    // now it will trigger again
    a.sampleTicksElapsed(20);
    assertEQ(host->gateChangeCount, 3);
    assert(host->gateState[0]);
    assertEQ(host->cvValue[0], 6);

    // after a lot of time it should shut off
    a.sampleTicksElapsed(44100);
    assertEQ(host->gateChangeCount, 4);
    assert(!host->gateState[0]);
    assertEQ(host->cvValue[0], 6);
}

static void testMultiNoteRetrigger()
{
    const float sampleRate = 44100;
    const float sampleTime = 1.0f / sampleRate;
    std::shared_ptr<TestHost2> host = std::make_shared<TestHost2>();
    MidiAudition a(host);
    a.setSampleTime(sampleTime);
    a.enable(true);

    float samplesToStop = MidiAudition::noteDurationSeconds() / sampleTime;
    int notEnoughSamples = int(samplesToStop / 2.f);

    a.auditionNote(5);
    assertEQ(host->gateChangeCount, 1);
    assert(host->gateState[0]);
    assertEQ(host->cvValue[0], 5);

    a.sampleTicksElapsed(notEnoughSamples);
    assertEQ(host->gateChangeCount, 1);
    assert(host->gateState[0]);

    // audition note again, should re-trigger
    a.auditionNote(6);
    assertEQ(host->gateChangeCount, 2);
    assert(!host->gateState[0]);
    assertEQ(host->cvValue[0], 5);

    float retriggerSamples = MidiAudition::retriggerDurationSeconds() / sampleTime;
    notEnoughSamples = int(retriggerSamples - 10);
    a.sampleTicksElapsed(notEnoughSamples);
    assertEQ(host->gateChangeCount, 2);
    assert(!host->gateState[0]);
    assertEQ(host->cvValue[0], 5);

    // extra notes shouldn't do anything bad
    a.auditionNote(7);
    a.auditionNote(8);

    a.sampleTicksElapsed(20);
    assertEQ(host->gateChangeCount, 3);
    assert(host->gateState[0]);
    assertEQ(host->cvValue[0], 8);
}


static void testSuppressWhilePlaying()
{
    std::shared_ptr<TestHost2> host = std::make_shared<TestHost2>();
    MidiAudition a(host);
    a.enable(false);

    a.auditionNote(5);

    assertEQ(host->gateChangeCount, 0);
    assert(!host->gateState[0]);
    assertLT(host->cvValue[0], 5);
}

static void testStartPlayingStopsAudition()
{
    std::shared_ptr<TestHost2> host = std::make_shared<TestHost2>();
    MidiAudition a(host);
    a.enable(true);

    a.auditionNote(5);

    assertEQ(host->gateChangeCount, 1);
    assert(host->gateState[0]);

    a.enable(false);
    assertEQ(host->gateChangeCount, 2);
    assert(!host->gateState[0]);
}

static void testTwoNotesAtOnce()
{
    std::shared_ptr<TestHost2> host = std::make_shared<TestHost2>();
    MidiAudition a(host);
    a.enable(true);
    const float sampleRate = 44100;
    const float sampleTime = 1.0f / sampleRate;
    a.setSampleTime(sampleTime);

    a.auditionNote(5);
    assert(host->gateState[0]);
    // second time should re-trigger
    a.auditionNote(5);
    assert(!host->gateState[0]);
    
    int retrigSamples = 0;
    for (retrigSamples = 0; retrigSamples < 45000; ++retrigSamples) {
        a.sampleTicksElapsed(1);
        if (host->gateState[0]) {
            break;
        }
        assert(retrigSamples < 44000);
    }

    assertEQ(retrigSamples, 44);
}


void testAudition()
{
    testSelectNoteAuditions();
    testNotNoteNoAudition();
    testSelectThreeBothAudition();
    testSelectNoteTwiceAuditionsOnce();
    testSelectNoteTwiceAuditionsOnce2();

    testPlaysNote();
    testNoteStops();
    testNoteRetrigger();
    testMultiNoteRetrigger();

    testSuppressWhilePlaying();
    testStartPlayingStopsAudition();
    testTwoNotesAtOnce();
}