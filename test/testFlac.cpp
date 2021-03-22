
#include "FilePath.h"
#include "FlacReader.h"
#include "SqLog.h"
#include "asserts.h"

static void test0() {
    FlacReader r;
    r.read(FilePath());
}

static void testWithFlac(const FilePath& f) {
    FlacReader r;
    r.read(f);
    assert(r.ok());

    SQINFO("file: %s", f.toString().c_str());

    for (int i = 0; i < 10; ++i) {
        const float s = r.getSamples()[i];
        SQINFO("sample[%d] =%f", i, s);
    }

    float x = -100;
    float y = 100;
    for (int i = 0; i < r.getNumSamples(); ++i) {
        const float d = r.getSamples()[i];
        x = std::max(x, d);
        y = std::min(y, d);
    }
    SQINFO("min = %f, max = %f", y, x);
    assertClose(x, 1, .0001);
    assertClose(y, -1, .0001);
}

static void testMono16() {
    testWithFlac(FilePath("D:\\samples\\test\\flac\\mono16.flac"));
}

static void testStereo16() {
    testWithFlac(FilePath("D:\\samples\\test\\flac\\stereo16.flac"));
}

static void testMono24() {
    testWithFlac(FilePath("D:\\samples\\test\\flac\\mono24.flac"));
}

static void testStereo24() {
    testWithFlac(FilePath("D:\\samples\\test\\flac\\stereo24.flac"));
}

 const char* path = R"foo(D:\samples\Ivy_Audio_Piano_in_162_sfz\Ivy Audio - Piano in 162 SFZ\Piano in 162 Samples\Ambient\PedalOffAmbient\51-PedalOffForte2Ambient.flac)foo";
 static void testPiano() {
     FlacReader r;
     r.read(FilePath(path));
     assert(r.ok());
   
 }


void testFlac() {
    test0();
    testMono16();
    testStereo16();
    testMono24();
    testStereo24();
    testPiano();
}