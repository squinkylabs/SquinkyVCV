
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

        MidiEvent ev;
        track->insertEvent(ev);
        assertEvCount(2);
    }
    assertEvCount(0);
}

static void testEventAccess()
{
    MidiSongPtr song(std::make_shared<MidiSong>());


    song->createTrack(0);
    auto track = song->getTrack(0);

    MidiEvent ev;
    ev.startTime = 100;
    ev.pitch = 40;
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
    auto x = i->second.startTime;
    its.first++;

}


static void testEventFilter()
{
    MidiSongPtr song(std::make_shared<MidiSong>());


    song->createTrack(0);
    auto track = song->getTrack(0);

    MidiEvent ev;
    ev.startTime = 100;
    ev.pitch = 40;
    track->insertEvent(ev);
    ev.startTime = 102;
    ev.pitch = 50;
    track->insertEvent(ev);
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
