


#include "Seq4.h"
#include "asserts.h"

extern MidiSong4Ptr makeTestSong4(int trackNum);


using Sq4 = Seq4<TestComposite>;

// TODO: move to a general UTIL
template <typename T>
static void initParams(T* composite)
{
    auto icomp = composite->getDescription();
    for (int i = 0; i < icomp->getNumParams(); ++i) {
        auto param = icomp->getParam(i);
        composite->params[i].value = param.def;
    }
}

/**
 * adapted from one in testSeqComposite
 * @param clockDiv - 4 for quarter, etc..
 */

std::shared_ptr<Sq4> make(SeqClock::ClockRate rate,
    int numVoices,
    bool toggleStart,
    int trackNum)
{
    assert(numVoices > 0 && numVoices <= 16);

    std::shared_ptr <MidiSong4> song = makeTestSong4(trackNum);

    auto ret = std::make_shared<Sq4>(song);

    // we SHOULD init the params properly for all the tests,
    // but not all work. this is a start.
    if (!toggleStart) {
        initParams(ret.get());
    }


    const float f = ret->params[Sq4::RUNNING_PARAM].value;

    ret->params[Sq4::NUM_VOICES_PARAM].value = float(numVoices - 1);
    ret->params[Sq4::CLOCK_INPUT_PARAM].value = float(rate);
    ret->inputs[Sq4::CLOCK_INPUT].setVoltage(0, 0);        // clock low
    if (toggleStart) {
        ret->toggleRunStop();                          // start it
    }

    return ret;
}



static void test0()
{
    std::shared_ptr<Sq4> comp = make(SeqClock::ClockRate::Div64, 4, false, 0);
}


void testSeqComposite4()
{

    test0();
}