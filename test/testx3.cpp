
#include "asserts.h"
#include "VelSwitch.h"
#include "SParse.h"

static void testVelSwitch1()
{
    VelSwitch v;

    SRegionPtr sr = std::make_shared<SRegion>();
    sr->compiledValues = SamplerSchema::compile(sr->values);
    CompiledRegionPtr r0 = std::make_shared<CompiledRegion>(sr);

    // Need to make some "test" sample playback ptrs. just need to be able to recognize them later
    ISamplerPlaybackPtr p0 = std::make_shared<SimpleVoicePlayer>(r0, 100, 0);
    ISamplerPlaybackPtr p1 = std::make_shared<SimpleVoicePlayer>(r0, 101, 0);
    ISamplerPlaybackPtr p2 = std::make_shared<SimpleVoicePlayer>(r0, 102, 0);
    ISamplerPlaybackPtr p3 = std::make_shared<SimpleVoicePlayer>(r0, 103, 0);

    v.addVelocityRange(0, p0);          // index zero starts at vel 0
    v.addVelocityRange(1, p1);
    v.addVelocityRange(99, p2);
    v.addVelocityRange(100, p3);

    VoicePlayInfo info;
    ISamplerPlaybackPtr test = v.mapVelToPlayer(0);
    test->play(info, 0, 0);
    assertEQ(info.sampleIndex, 100);

    test = v.mapVelToPlayer(1);
    test->play(info, 0, 0);
    assertEQ(info.sampleIndex, 101);

    test = v.mapVelToPlayer(99);
    test->play(info, 0, 0);
    assertEQ(info.sampleIndex, 102);

    test = v.mapVelToPlayer(100);
    test->play(info, 0, 0);
    assertEQ(info.sampleIndex, 103);
}

void testx3()
{
    testVelSwitch1();
}