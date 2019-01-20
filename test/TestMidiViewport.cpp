
#include "asserts.h"
#include "MidiViewport.h"

#include "MidiSong.h"


static void testReleaseSong()
{
    MidiSongPtr song(std::make_shared<MidiSong>());
    MidiViewport vp(song);
    {
       
       // vp._song = song;

        song->createTrack(0);
        auto track = song->getTrack(0);

        MidiNoteEventPtr ev = std::make_shared<MidiNoteEvent>();
        track->insertEvent(ev);
        assertEvCount(1);       // one obj with two refs
    }
    song.reset();       // give up the one strong ref
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

    MidiViewport vp(song);
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

    MidiViewport vp(song);
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


    MidiViewport viewport(song);
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

    // try inclusive pitch range
    viewport.pitchLow = PitchUtils::pitchToCV(3, 0);     // pitch of first note
    viewport.pitchHi = PitchUtils::pitchToCV(3, 7);
    it = viewport.getEvents();
    assertEQ(std::distance(it.first, it.second), numNotes);

    // reduce the pitch range to lose the highest note.
    viewport.pitchHi = PitchUtils::pitchToCV(3, 7) - .01f;
    it = viewport.getEvents();
    assertEQ(std::distance(it.first, it.second), numNotes-1);

     // reduce the pitch range to lose the lowest note.
    viewport.pitchLow = PitchUtils::pitchToCV(3, 0) + .01f;
    it = viewport.getEvents();
    assertEQ(std::distance(it.first, it.second), numNotes - 2);

    // try inclusive pitch range, but only half the time
    viewport.pitchLow = PitchUtils::pitchToCV(3, 0);     // pitch of first note
    viewport.pitchHi = PitchUtils::pitchToCV(3, 7);
    viewport.endTime = viewport.startTime + 4;   // one measures
    it = viewport.getEvents();
    assertEQ(std::distance(it.first, it.second), numNotes /2);



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
