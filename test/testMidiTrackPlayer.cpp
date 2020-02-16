
#include "MidiSong4.h"
#include "MidiTrackPlayer.h"
#include "TestHost2.h"
#include "asserts.h"

static void testCanCall()
{
    std::shared_ptr<IMidiPlayerHost4> host = std::make_shared<TestHost2>();
    std::shared_ptr<MidiSong4> song = std::make_shared<MidiSong4>();
    MidiTrackPlayer pl(host, 0, song);

    const float quantizationInterval = .01f;
    bool b = pl.playOnce(1, quantizationInterval);
    assert(!b);
}

void testMidiTrackPlayer()
{
    testCanCall();
   
}