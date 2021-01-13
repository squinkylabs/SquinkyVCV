
#include "SqLog.h"
#include "asserts.h"
#include "samplerTests.h"

static void testRandomPlayerEmpty() {
    VoicePlayInfo info;
    VoicePlayParameter params;
    RandomVoicePlayerPtr player = std::make_shared<RandomVoicePlayer>();
    player->finalize();
    player->play(info, params);
    assert(!info.valid);
}

/*
  VoicePlayInfo(CompiledRegionPtr region, int midiPitch, int sampleIndex);
    bool valid = false;
    int sampleIndex = 0;
    bool needsTranspose = false;
    float transposeAmt = 1;
    float gain = 1;             // assume full volume
    float ampeg_release = .001f;

*/

static void testRandomPlayerOneEntry() {
    CompiledRegionPtr cr = st::makeRegion(R"foo(<region>sample=a lorand=0 hirand=1)foo");

    VoicePlayInfo info;
    VoicePlayParameter params;
    params.midiVelocity = 20;
    params.midiPitch = 60;
    RandomVoicePlayerPtr player = std::make_shared<RandomVoicePlayer>();
    player->addEntry(cr, 1, 60);
    player->finalize();
    player->play(info, params);
    assert(info.valid);
    assertEQ(info.sampleIndex, 1);
}

static void testRandomPlayerTwoEntrySub(const char* reg1, const char* reg2, bool flipped) {
    CompiledRegionPtr cr1 = st::makeRegion(reg1);
    CompiledRegionPtr cr2 = st::makeRegion(reg2);

    VoicePlayInfo info;
    VoicePlayParameter params;
    params.midiPitch = 60;
    params.midiVelocity = 100;
    RandomVoicePlayerPtr player = std::make_shared<RandomVoicePlayer>();

    player->addEntry(cr1, 100, 60);
    player->addEntry(cr2, 101, 60);
    player->finalize();
    int ct100 = 0;
    int ct101 = 0;

    for (int i = 0; i < 100; ++i) {
        player->play(info, params);
        assert(info.valid);
        switch (info.sampleIndex) {
            case 100:
                ct100++;
                break;
            case 101:
                ct101++;
                break;
            default:
                assert(false);
        }
    }
    if (!flipped) {
        assertClose(ct100, 20, 10);
        assertClose(ct101, 80, 10);
    } else {
        assertClose(ct101, 20, 10);
        assertClose(ct100, 80, 10);
    }
}

static void testRandomPlayerTwoEntry() {
    SQINFO("testRandomPlayerTwoEntry");
    testRandomPlayerTwoEntrySub(R"foo(<region>sample=a lorand=0 hirand=.2)foo",
                                R"foo(<region>sample=a lorand=.2 hirand=1)foo",
                                false);
}

static void testRandomPlayerTwoEntryB() {
    SQINFO("testRandomPlayerTwoEntrySubB");
    testRandomPlayerTwoEntrySub(
        R"foo(<region>sample=a lorand=.2 hirand=1)foo",
        R"foo(<region>sample=a lorand=0 hirand=.2)foo",
        true);
}

static void testRandomPlayerBadData() {
    SQINFO("test bad data");
    CompiledRegionPtr cr1 = st::makeRegion(R"foo(<region>sample=a lorand=.2 hirand=100)foo");
    CompiledRegionPtr cr2 = st::makeRegion(R"foo(<region>sample=a lorand=.2 hirand=1)foo");

    VoicePlayInfo info;
    VoicePlayParameter params;
    params.midiPitch = 60;
    params.midiVelocity = 100;
    RandomVoicePlayerPtr player = std::make_shared<RandomVoicePlayer>();

    player->addEntry(cr1, 100, 60);
    player->addEntry(cr2, 101, 60);
    player->finalize();
    int ct100 = 0;
    int ct101 = 0;

    for (int i = 0; i < 100; ++i) {
        player->play(info, params);
        assert(info.valid);
        switch (info.sampleIndex) {
            case 100:
                ct100++;
                break;
            case 101:
                ct101++;
                break;
            default:
                assert(false);
        }
    }
    // on bad data we ignore probabilities
    assertClose(ct101, 50, 10);
    assertClose(ct100, 50, 10); 
}


static void testRRPlayerCrazyPos() {
    CompiledRegionPtr cr1 = st::makeRegion(R"foo(<region>sample=a seq_position=200)foo");
    CompiledRegionPtr cr2 = st::makeRegion(R"foo(<region>sample=b  seq_position=10)foo");
    CompiledRegionPtr cr3 = st::makeRegion(R"foo(<region>sample=c  seq_position=2000)foo");

     VoicePlayInfo info;
    VoicePlayParameter params;
    params.midiPitch = 60;
    params.midiVelocity = 100;
    RoundRobinVoicePlayerPtr player = std::make_shared<RoundRobinVoicePlayer>();

    player->addEntry(cr1, 101, 60);
    player->addEntry(cr2, 102, 60);
    player->addEntry(cr3, 103, 60);
    player->finalize();

    player->play(info, params);
    assert(info.valid);
    assertEQ(info.sampleIndex, 102);        // b is lowest seq

    player->play(info, params);
    assert(info.valid);
    assertEQ(info.sampleIndex, 101);        // a is second lowest seq

    player->play(info, params);
    assert(info.valid);
    assertEQ(info.sampleIndex, 103);        //c is highest

    player->play(info, params);
    assert(info.valid);
    assertEQ(info.sampleIndex, 102);        // b is lowest seq

}

void testx4() {
    testRandomPlayerEmpty();
    testRandomPlayerOneEntry();
    testRandomPlayerTwoEntry();
    testRandomPlayerTwoEntryB();
    testRandomPlayerBadData();

    testRRPlayerCrazyPos();
}