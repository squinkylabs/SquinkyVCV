
#include "asserts.h"
#include "MidiViewport.h"

#include "MidiSong.h"


static void testReleaseSong()
{
    MidiViewport vp;
    {
        MidiSongPtr song(std::make_shared<MidiSong>());
        vp._song = song;

        song->createTrack(0);
        auto track = song->getTrack(0);

        MidiEventPtr ev = std::make_shared<MidiEvent>();
        track->insertEvent(ev);
        assertEvCount(1);       // one obj with two refs
    }
    assertEvCount(0);
}

static void testEventAccess()
{
    MidiSongPtr song(std::make_shared<MidiSong>());


    song->createTrack(0);
    auto track = song->getTrack(0);

    MidiEventPtr ev = std::make_shared<MidiEvent>();
    ev->startTime = 100;
    ev->pitch = 40;
    track->insertEvent(ev);

    MidiViewport vp;
    vp._song = song;
    vp.startTime = 90;
    vp.endTime = 110;
    vp.pitchLow = 0;
    vp.pitchHi = 80;

    auto its = vp.getEvents();
    assertEQ(std::distance(its.first, its.second), 1);

    assert(its.first != its.second);

    auto i = its.first;
    auto x = i->second->startTime;
    its.first++;
}


static void testEventFilter()
{
    MidiSongPtr song(std::make_shared<MidiSong>());


    song->createTrack(0);
    auto track = song->getTrack(0);

    MidiEventPtr ev = std::make_shared<MidiEvent>();
    ev->startTime = 100;
    ev->pitch = 40;
    track->insertEvent(ev);

    MidiEventPtr ev2 = std::make_shared<MidiEvent>();
    ev2->startTime = 102;
    ev2->pitch = 50;
    ev2->startTime = 100;
    track->insertEvent(ev2);
    assertEQ(track->size(), 2);

    MidiViewport vp;
    vp._song = song;
    vp.startTime = 90;
    vp.endTime = 110;
    vp.pitchLow = 3;
    vp.pitchHi = 45;
    auto its = vp.getEvents();
    assertEQ(std::distance(its.first, its.second), 1);
}

void testMidiViewport()
{

    assertEvCount(0);
    testReleaseSong();
    testEventAccess();
    testEventFilter();

    assertEvCount(0);
}
