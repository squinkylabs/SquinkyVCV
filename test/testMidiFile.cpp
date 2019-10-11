
#include "MidiSong.h"
#include "MidiTrack.h"
#include "MidiFileProxy.h"
#include "asserts.h"
#include <filesystem>
#include <stdio.h>

static void test1()
{
#if defined(_MSC_VER)
    const char* path = "..\\..\\test\\test1.mid";
#else
    const char* path = "./test/test1.mid";
#endif
    MidiSongPtr song = MidiFileProxy::load(path);

    fflush(stdout);
    assert(song);
    assertEQ(song->getHighestTrackNumber(), 0);

    MidiTrackPtr track = song->getTrack(0);
    assertEQ(track->size(), 3);
}

static void test2()
{
    char buffer[FILENAME_MAX];
    auto ret = tmpnam_s(buffer, FILENAME_MAX);

    printf("temp file = %s\n", buffer);
    MidiSongPtr song = MidiSong::makeTest(MidiTrack::TestContent::FourAlmostTouchingQuarters, 0);
    bool b = MidiFileProxy::save(song, buffer);
    assert(b);

    MidiSongPtr song2 = MidiFileProxy::load(buffer);
    assert(song2);
    assertEQ(song2->getTrack(0)->size(), song->getTrack(0)->size());
}
void testMidiFile()
{
    test1();
    test2();
}