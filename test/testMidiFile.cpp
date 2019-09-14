
#include "MidiSong.h"
#include "MidiFileProxy.h"
#include "asserts.h"

static void test1()
{
#if defined(_MSC_VER)
    const char* path = "..\\..\\test\\test1.mid";
#else
    const char* path = ".\\test\\test1.mid";
#endif
    MidiSongPtr song = MidiFileProxy::load(path);

    fflush(stdout);
    assert(song);
    assertEQ(song->getHighestTrackNumber(), 0);

    MidiTrackPtr track = song->getTrack(0);
    assertEQ(track->size(), 3);
}

void testMidiFile()
{
    test1();
}