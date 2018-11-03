
#include <assert.h>
#include "MidiTrack.h"
#include "asserts.h"



static void testCanInsert()
{
    MidiTrack mt;
    MidiEvent ev;
    ev.pitch = 33;
    ev.startTime = 55;
    assert(mt.size() == 0);
    mt.insertEvent(ev);
    assert(mt.size() == 1);

    MidiEvent ev2 = mt._testGetVector()[0];
    assert(ev2 == ev);

    assert(mt.isValid());
}

static void testInsertSorted()
{
    MidiTrack mt;
    MidiEvent ev;
    ev.pitch = 33;
    ev.startTime = 11;

    MidiEvent ev2;
    ev2.pitch = 44;
    ev2.startTime = 1;

    mt.insertEvent(ev);
    mt.insertEvent(ev2);

    auto mv = mt._testGetVector();
    MidiEvent& ev3 = mv.at(0);
    assert(ev3 == ev2);

    ev3 = mv.at(1);
    assert(ev3 == ev);

    assert(mt.isValid());
}

static void testDelete()
{
    MidiTrack mt;
    MidiEvent ev;
    ev.pitch = 33;
    ev.startTime = 11;

    mt.insertEvent(ev);
    mt.deleteEvent(ev);
    assert(mt.size() == 0);
}

static void testDelete2()
{
    MidiTrack mt;
    MidiEvent ev;

    ev.pitch = 33;
    ev.startTime = 11;
    mt.insertEvent(ev);

    ev.pitch = 44;
    mt.insertEvent(ev);

    mt.deleteEvent(ev);     // delete the second one, with pitch 44
    auto mv = mt._testGetVector();

    assert(mt.size() == 1);
    assert(mv[0].pitch == 33);
}

static void testDelete3()
{
    MidiTrack mt;
    MidiEvent ev;

#ifdef _DEBUG
    assert(MidiEvent::_count > 0);
#endif
    ev.pitch = 44;
    ev.startTime = 11;
    mt.insertEvent(ev);

    ev.pitch = 33;
    mt.insertEvent(ev);

    ev.pitch = 44;
    mt.deleteEvent(ev);     // delete the first one, with pitch 44
    auto mv = mt._testGetVector();

    assert(mt.size() == 1);
    assert(mv[0].pitch == 33);
}

static void testTimeRange0()
{
    MidiTrack mt;
    MidiEvent ev;
    ev.startTime = 100;

    mt.insertEvent(ev);
    
    MidiTrack::iterator_pair its = mt.timeRange(99, 101);
    assert(its.first != its.second);
    auto count = std::distance(its.first, its.second);
    assertEQ(count, 1);

    its = mt.timeRange(101, 1000);
    assert(its.first == its.second);
    count = std::distance(its.first, its.second);
    assertEQ(count, 0);
}

static void testTimeRange1()
{
    MidiTrack mt;
    MidiEvent ev;

    ev.startTime = 100;
    mt.insertEvent(ev);
    ev.startTime = 110;
    mt.insertEvent(ev);
    ev.startTime = 120;
    mt.insertEvent(ev);
    ev.startTime = 130;
    mt.insertEvent(ev);

    MidiTrack::iterator_pair its = mt.timeRange(110, 120);
    assert(its.first != its.second);
    auto count = std::distance(its.first, its.second);
    assertEQ(count, 2);
}

void testMidiDataModel()
{
    assertEvCount(0);
    testCanInsert();
    testInsertSorted();
    testDelete();
    testDelete2();
    testDelete3();
    testTimeRange0();
    testTimeRange1();
    assertEvCount(0);
}