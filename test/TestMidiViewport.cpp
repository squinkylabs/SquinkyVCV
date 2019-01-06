
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

        MidiNoteEventPtr ev = std::make_shared<MidiNoteEvent>();
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

    MidiNoteEventPtr ev = std::make_shared<MidiNoteEvent>();
    ev->startTime = 100;
    ev->pitchCV = 40;
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

    MidiNoteEventPtr ev = std::make_shared<MidiNoteEvent>();
    ev->startTime = 100;
    ev->pitchCV = 40;
    track->insertEvent(ev);

    MidiNoteEventPtr ev2 = std::make_shared<MidiNoteEvent>();
    ev2->startTime = 102;
    ev2->pitchCV = 50;
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

static void testDemoSong()
{
    MidiSongPtr song = MidiSong::makeTest1();
    MidiTrackPtr track = song->getTrack(0);

    const int numNotes = 8;             // track0 of test song has 8 notes
    const int numEvents = 8 + 1;        // got an end event
    assertEQ(std::distance(track->begin(), track->end()), numEvents);


    MidiViewport viewport;
    viewport._song = song;
    viewport.startTime = 0;
    viewport.endTime = viewport.startTime + 8;   // two measures

    // try a crazy wide range
    MidiNoteEvent note;
    note.setPitch(-10, 0);

    viewport.pitchLow =  note.pitchCV;
    note.setPitch(10, 0);
    viewport.pitchHi =  note.pitchCV;
 
    MidiViewport::iterator_pair it = viewport.getEvents();
    assertEQ(std::distance(it.first, it.second), numNotes);
}

void testMidiViewport()
{

    assertEvCount(0);
    testReleaseSong();
    testEventAccess();
    testEventFilter();
    testDemoSong();

    assertEvCount(0);
}
