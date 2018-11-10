#include "MidiPlayer.h"
#include "MidiSong.h"


class TestHost : public MidiPlayer::IPlayerHost
{
public:
    void setGate(bool) override
    {

    }
    void setCV(float) override
    {

    }
};


static void test0()
{
    std::shared_ptr<MidiSong> song = MidiSong::makeTest1();
    std::shared_ptr<TestHost> host = std::make_shared<TestHost>();
    MidiPlayer pl(host, song);
    pl.timeElapsed(.01f);
}


void testMidiPlayer()
{
    test0();
}