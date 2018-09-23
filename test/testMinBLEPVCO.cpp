#include <assert.h>

#include "asserts.h"
#include "EV3.h"
#include "MinBLEPVCO.h"
#include "TestComposite.h"

static float sampleTime = 1.0f / 44100.0f;


class TestMB
{
public:
    //static void  setAllWaveforms(MinBLEPVCO* vco);
   // static void test1();
    static void testSync2();
    static void setPitch(EV3<TestComposite>& ev3);
};

#if 0
// puts non-zero in all the waveforms
 void TestMB::setAllWaveforms(MinBLEPVCO* vco)
{
  //  float * wave = vco->_getWaveforms();
    for (int i = 0; i < (int)MinBLEPVCO::Waveform::END; ++i) {
        vco->waveformOutputs[i] = 1;
    }
}
#endif

#if 0
void TestMB::test1()
{
    MinBLEPVCO vco;
    setAllWaveforms(&vco);
    assertNE(vco.getWaveform(MinBLEPVCO::Waveform::Saw), 0);
    assertNE(vco.getWaveform(MinBLEPVCO::Waveform::Sin), 0);
    assertNE(vco.getWaveform(MinBLEPVCO::Waveform::Square), 0);
    assertNE(vco.getWaveform(MinBLEPVCO::Waveform::Tri), 0);
    assertNE(vco.getWaveform(MinBLEPVCO::Waveform::Even), 0);

 
    vco.zeroOutputsExcept(MinBLEPVCO::Waveform::Saw);
    assertNE(vco.getWaveform(MinBLEPVCO::Waveform::Saw), 0);
    assertEQ(vco.getWaveform(MinBLEPVCO::Waveform::Sin), 0);
    assertEQ(vco.getWaveform(MinBLEPVCO::Waveform::Square), 0);
    assertEQ(vco.getWaveform(MinBLEPVCO::Waveform::Tri), 0);
    assertEQ(vco.getWaveform(MinBLEPVCO::Waveform::Even), 0);

    // special case for even and sin
    setAllWaveforms(&vco);
    vco.zeroOutputsExcept(MinBLEPVCO::Waveform::Even);
    assertEQ(vco.getWaveform(MinBLEPVCO::Waveform::Saw), 0);
    assertNE(vco.getWaveform(MinBLEPVCO::Waveform::Sin), 0);
    assertEQ(vco.getWaveform(MinBLEPVCO::Waveform::Square), 0);
    assertEQ(vco.getWaveform(MinBLEPVCO::Waveform::Tri), 0);
    assertNE(vco.getWaveform(MinBLEPVCO::Waveform::Even), 0);

    setAllWaveforms(&vco);
    vco.zeroOutputsExcept(MinBLEPVCO::Waveform::Square);
    assertEQ(vco.getWaveform(MinBLEPVCO::Waveform::Saw), 0);
    assertEQ(vco.getWaveform(MinBLEPVCO::Waveform::Sin), 0);
    assertNE(vco.getWaveform(MinBLEPVCO::Waveform::Square), 0);
    assertEQ(vco.getWaveform(MinBLEPVCO::Waveform::Tri), 0);
    assertEQ(vco.getWaveform(MinBLEPVCO::Waveform::Even), 0);
}


static void test0()
{
    MinBLEPVCO vco;

    // Don't enable any waveforms
   // vco.setSampleTime(sampleTime);
    vco.setNormalizedFreq(1000 * sampleTime);
    vco.step();

    // should get nothing out.
    assert(vco.getWaveform(MinBLEPVCO::Waveform::Sin) == 0);
    assert(vco.getWaveform(MinBLEPVCO::Waveform::Square) == 0);
    assert(vco.getWaveform(MinBLEPVCO::Waveform::Saw) == 0);
    assert(vco.getWaveform(MinBLEPVCO::Waveform::Tri) == 0);
    assert(vco.getWaveform(MinBLEPVCO::Waveform::Even) == 0);
}
#endif


static void testSaw1()
{
    MinBLEPVCO vco;

    vco.setNormalizedFreq(1000 * sampleTime);
    vco.enableWaveform(MinBLEPVCO::Waveform::Saw, true);
    vco.step();

    // should get saw out.
  //  assertEQ(vco.getWaveform(MinBLEPVCO::Waveform::Sin), 0);
   // assertEQ(vco.getWaveform(MinBLEPVCO::Waveform::Square), 0);
  //  assertNE(vco.getWaveform(MinBLEPVCO::Waveform::Saw), 0);
   // assertEQ(vco.getWaveform(MinBLEPVCO::Waveform::Tri), 0);
   // assertEQ(vco.getWaveform(MinBLEPVCO::Waveform::Even), 0);
    assertNE(vco.getWaveform(), 0);

}

static void testSync1()
{
    MinBLEPVCO vco;

    vco.setNormalizedFreq(1000 * sampleTime);
    vco.enableWaveform(MinBLEPVCO::Waveform::Saw, true);
    float lastOut = -1000;
    vco.step();

    // first make sure it's going up.
    for (int i = 0; i < 10; ++i) {
        vco.step();
        const float x = vco.getWaveform();
        assertGT(x, lastOut);
        lastOut = x;
    }

    vco.onMasterSync(10, -2);       // set a reset to VCO
    vco.step();
    const float x = vco.getWaveform();
    assertLT(x, lastOut);
}

void TestMB::setPitch(EV3<TestComposite>& ev3)
{
    ev3.params[EV3<TestComposite>::OCTAVE1_PARAM].value = 2;
    ev3.params[EV3<TestComposite>::OCTAVE2_PARAM].value = 3;
    ev3.params[EV3<TestComposite>::OCTAVE3_PARAM].value = 3;

    // raise 2,3 by an oct and a semitone
    ev3.params[EV3<TestComposite>::SEMI1_PARAM].value = 0;
    ev3.params[EV3<TestComposite>::SEMI2_PARAM].value = 1;
    ev3.params[EV3<TestComposite>::SEMI3_PARAM].value = 1;

    ev3.vcos[0].enableWaveform(MinBLEPVCO::Waveform::Saw, true);
    ev3.vcos[1].enableWaveform(MinBLEPVCO::Waveform::Saw, true);
    ev3.vcos[2].enableWaveform(MinBLEPVCO::Waveform::Saw, true);


}

void TestMB::testSync2()
{
    printf("***** testSync2*****\n");
    EV3<TestComposite> ev3;
    setPitch(ev3);

    ev3.step();
    const float f0 = ev3._freq[0];
    const float f1 = ev3._freq[1];
    
    assertClose(f0, 2093.02, .005);
    assertClose(f1, 4434.95, .005);

    float last0 = -10;
    float last1 = -10;
    for (int i = 0; i < 100; ++i) {
        ev3.step();
 
        //printf("phase==%.2f phase1==%.2f ", ev3.vcos[0].phase, ev3.vcos[1].phase);
        float x = ev3._out[0];
       // assert(x > last0);
        //printf("%d delta0=%.2f",i, x - last0);
        last0 = x;

        x = ev3._out[1];
       // assert(x > last1);
       
        printf(" delta1=%.2f", x - last1);
        printf(" 0=%.2f 1=%.2f\n", last0, last1);
        fflush(stdout);
        last1 = x;
    }

    // TODO: test the sync on/off
  
}
void testMinBLEPVCO()
{
    // A lot of these tests are from old API
//    TestMB::test1();
  //  printf("fix the minb tests\n");
  //  test0();


    testSaw1();
    testSync1();

    // this one doesn't work, either.
    //TestMB::testSync2();
}