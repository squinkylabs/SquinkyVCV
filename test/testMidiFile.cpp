
#include "MidiSong.h"
#include "MidiTrack.h"
#include "MidiFileProxy.h"
#include "asserts.h"
//#include <filesystem>
#define __STDC_WANT_LIB_EXT1__ 1        // to get tempnam_s
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

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


static void compareSongs(MidiSongPtr song1, MidiSongPtr song2)
{
    assertEQ(song2->getTrack(0)->size(), song1->getTrack(0)->size());
    assertEQ(song1->getHighestTrackNumber(), 0);
    assertEQ(song2->getHighestTrackNumber(), 0);

    MidiTrack::const_iterator it = song1->getTrack(0)->begin();
    MidiTrack::const_iterator it2 = song2->getTrack(0)->begin();
    while (it != song1->getTrack(0)->end()) {
        assert(it2 != song2->getTrack(0)->end());

        MidiEventPtr ev1 = it->second;
        MidiEventPtr ev2 = it2->second;

        if (ev1->type == MidiEvent::Type::End) {
            assert(ev2->type == MidiEvent::Type::End);
            assertEQ(ev1->startTime, ev2->startTime);
        } else {
            MidiNoteEventPtr note1 = safe_cast<MidiNoteEvent>(ev1);
            MidiNoteEventPtr note2 = safe_cast<MidiNoteEvent>(ev2);
            assert(note2 && note1);
            assertEQ(note1->startTime, note2->startTime);
            assertEQ(note1->pitchCV, note2->pitchCV);
        }
        ++it;
        ++it2;
    }
}

static void test2()
{
    char buffer[FILENAME_MAX];
    auto ret = tmpnam_s(buffer, FILENAME_MAX);
    assert(!ret);

    printf("temp file = %s\n", buffer);
    MidiSongPtr song = MidiSong::makeTest(MidiTrack::TestContent::FourAlmostTouchingQuarters, 0);
    bool b = MidiFileProxy::save(song, buffer);
    assert(b);

    MidiSongPtr song2 = MidiFileProxy::load(buffer);
    struct stat filestatus;
    stat(buffer, &filestatus);
    assertGT(filestatus.st_size, 50);
    remove(buffer);
    assert(song2);
    compareSongs(song, song2);
}

void testMidiFile()
{
    test1();
    test2();
}