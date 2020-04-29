


#include "Seq4.h"
#include "asserts.h"

extern MidiSong4Ptr makeTestSong4(int trackNum);


using Sq4 = Seq4<TestComposite>;
using Sq4Ptr = std::shared_ptr<Sq4>;

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



MidiSong4Ptr makeTestSongAll()
{
    MidiSong4Ptr song = std::make_shared<MidiSong4>();
    MidiLocker lock(song->lock);
    MidiTrackPtr clip = MidiTrack::makeTest(MidiTrack::TestContent::eightQNotesCMaj, song->lock);

    // same clipt everywhere is a little hinky, but will work for this test.
    for (int i = 0; i < 4; ++i) {
        song->addTrack(i, 0, clip);
        song->addTrack(i, 1, clip);
        song->addTrack(i, 2, clip);
        song->addTrack(i, 3, clip);
    }
    return song;
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
    // assert(trackNum >= 0);

    std::shared_ptr <MidiSong4> song = (trackNum >= 0) ? makeTestSong4(trackNum) : makeTestSongAll();

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

static void stepN(Sq4Ptr sq, int numTimes)
{
    for (int i = 0; i < numTimes; ++i) {
        sq->step();
    }
}

static void genOneClock(Sq4Ptr sq)
{
    sq->inputs[Sq4::CLOCK_INPUT].setVoltage(10, 0);
    stepN(sq, 16);
    sq->inputs[Sq4::CLOCK_INPUT].setVoltage(0, 0);
    stepN(sq, 16);
}

static void play(std::shared_ptr<Sq4> comp, SeqClock::ClockRate rate, float quarterNotes)
{
    assert(rate == SeqClock::ClockRate::Div64);
    const int clocks = int(64.f * quarterNotes);
    for (int i = 0; i < clocks; ++i) {
        genOneClock(comp);
    }
}

// test seq is 1,2,2,2 bars
// very basic test to make sure our scffolding works
static void test0()
{
    const int tkNum = 0;
    const auto rate = SeqClock::ClockRate::Div64;
    Sq4Ptr comp = make(rate, 4, true, tkNum);
    MidiTrackPlayerPtr pl = comp->getTrackPlayer(tkNum);

    stepN(comp, 16);
    assertEQ(pl->_getRunningStatus(), true);


    // play to third quarter note
    play(comp, rate, 3.f);
    assertEQ(pl->getSection(), 1);      // first section is 1

    // play just past start of next section
    play(comp, rate, 1.1f);
    assertEQ(pl->getSection(), 2); 
}



// test seq is 1,2,2,2 bars
static void testPause()
{
    const int tkNum = 0;
    const auto rate = SeqClock::ClockRate::Div64;
    Sq4Ptr comp = make(rate, 4, true, tkNum);
    MidiTrackPlayerPtr pl = comp->getTrackPlayer(tkNum);

    stepN(comp, 16);
    assertEQ(pl->_getRunningStatus(), true);

    // play to third quarter note
    play(comp, rate, 3.f);
    assertEQ(pl->getSection(), 1);      // first section is 1

    comp->toggleRunStop();              // pause it
    stepN(comp, 16);
    assertEQ(pl->_getRunningStatus(), false);

    play(comp, rate, 2.f);              // would be section 2, if not paused.
    assertEQ(pl->getSection(), 1);
}


/*
Here's what's in the test song. number of bars is 1,2,2,2
MidiSong4Ptr makeTestSong4(int trackNum)
{
    MidiSong4Ptr song = std::make_shared<MidiSong4>();
    MidiLocker lock(song->lock);
    MidiTrackPtr clip0 = MidiTrack::makeTest(MidiTrack::TestContent::oneQ1_75, song->lock);
    MidiTrackPtr clip1 = MidiTrack::makeTest(MidiTrack::TestContent::eightQNotesCMaj, song->lock);
    MidiTrackPtr clip2 = MidiTrack::makeTest(MidiTrack::TestContent::eightQNotesCMaj, song->lock);
    MidiTrackPtr clip3 = MidiTrack::makeTest(MidiTrack::TestContent::eightQNotesCMaj, song->lock);

we are failing this test, becuase after we switch section, playback still seems to be playing
 section 0, instead of 3

 No, actually what is happening is that we are playing sec 3, but still at metric time == 3.
 If we pause and change sections, we porbably need to reset metric time.
*/
extern float lastTime;
static void testPauseSwitchSectionStart()
{
    lastTime = -100;

    const int tkNum = 0;
    const auto rate = SeqClock::ClockRate::Div64;
    Sq4Ptr comp = make(rate, 4, true, tkNum);
    MidiTrackPlayerPtr pl = comp->getTrackPlayer(tkNum);
    stepN(comp, 16);
    assertEQ(pl->_getRunningStatus(), true);

    // play to first q note at 1.0, at pitch 7.5
    play(comp, rate, 1.2f);
    assertEQ(pl->getSection(), 1);          // first section is 1                                      
    assertGT(comp->outputs[comp->GATE0_OUTPUT].getVoltage(0), 5);
    assertEQ(comp->outputs[comp->CV0_OUTPUT].getVoltage(0), 7.5f);

    // play to third quarter note of first pattern. there shouldn't be anything playing there.
    play(comp, rate, 3.f - 1.2f);
    assertEQ(pl->getSection(), 1);          // first section is 1
    assertLT(comp->outputs[comp->GATE0_OUTPUT].getVoltage(0), 5);

    comp->toggleRunStop();                  // pause it
    lastTime = -100;
    stepN(comp, 16);
    assertEQ(pl->_getRunningStatus(), false);

    comp->setNextSectionRequest(tkNum, 4);  // goto last section (#4)

    stepN(comp, 16);

    lastTime = -100;

    comp->toggleRunStop();                  // resume it

    lastTime = -100;
    stepN(comp, 16);

    lastTime = -100;
    assertEQ(pl->_getRunningStatus(), true);

    /* This issue here is that this test wants stepN to just call the track player's step() function to service the queue.
     * but really it's going to run the clock, also.
     */

    play(comp, rate, .1f);                  // play a tinnny bit to prime
    assertEQ(pl->getSection(), 4);          // should be in new section

    // should be playing the first note of the next section now. note that it
    // will have rotated to the next voice
    const float expectedPitch = PitchUtils::pitchToCV(3, PitchUtils::c);
    assertGT(comp->outputs[comp->GATE0_OUTPUT].getVoltage(1), 5);
    assertLT(comp->outputs[comp->GATE0_OUTPUT].getVoltage(0), 1);
    assertEQ(comp->outputs[comp->CV0_OUTPUT].getVoltage(1), expectedPitch);

    play(comp, rate, 5.f); // play most (this section 2 bars)
    assertEQ(pl->getSection(), 4);       // should be playing requested section still
}

 static void testSelectSectionWithCV(int cOctave, int sectionToSelect)
 {
    // printf("\n--- testSelectSectionWithCV oct=%d, selection=%d\n", cOctave, sectionToSelect);
    const auto rate = SeqClock::ClockRate::Div64;
    Sq4Ptr comp = make(rate, 4, true, -1);

    play(comp, rate, .1f);                  // play a tinny bit to prime

    // there should be no active track requests
    for (int i=0; i<MidiSong4::numTracks; ++i) {
        assertEQ(comp->getNextSectionRequest(i), 0);
    }

    comp->params[comp->CV_SELECT_OCTAVE_PARAM].value = float(cOctave);
    comp->inputs[comp->SELECT_CV_INPUT].channels = 1;
    comp->inputs[comp->SELECT_GATE_INPUT].channels = 1;
    comp->inputs[comp->SELECT_CV_INPUT].setVoltage(2, 0);
    comp->inputs[comp->SELECT_GATE_INPUT].setVoltage(10, 0);

    // set no gate in channel 0
    comp->inputs[comp->SELECT_CV_INPUT].setVoltage(-2, 0);
    comp->inputs[comp->SELECT_GATE_INPUT].setVoltage(0, 0);

    play(comp, rate, .2f);
    float cv = PitchUtils::pitchToCV(cOctave, sectionToSelect);
    const int row = sectionToSelect / MidiSong4::numTracks;
    const int col = sectionToSelect % MidiSong4::numTracks;
    //printf("in test row=%d col=%d\n", row, col);

    assert(comp->getSong()->getTrack(row, col));


    // gate a c4
    comp->inputs[comp->SELECT_CV_INPUT].setVoltage(cv, 0);
    comp->inputs[comp->SELECT_GATE_INPUT].setVoltage(10, 0);
    play(comp, rate, .3f);

    // now check that the correct section is requested

    for (int r = 0; r < MidiSong4::numTracks; ++r) {
        const int expectedRequest = (r == row) ? col + 1 : 0;
        assertEQ(comp->getNextSectionRequest(r), expectedRequest);
    }
 }

static void testSelectSectionWithCV()
{
    assertEQ(MidiSong4::numTracks, 4);
    assertEQ(MidiSong4::numSectionsPerTrack, 4);


   for (int octave = 0; octave < 10; ++ octave) {
        for (int i=0; i<16; ++i) {
            testSelectSectionWithCV(octave, i);
        }
    }

}

static void testSelectSectionWithCVPoly()
{
    printf("\n--- testSelectSectionWithCVPoly\n");
    const auto rate = SeqClock::ClockRate::Div64;
    Sq4Ptr comp = make(rate, 4, true, -1);

    play(comp, rate, .1f);                  // play a tinny bit to prime

    // there should be no active track requests
    for (int i = 0; i < MidiSong4::numTracks; ++i) {
        assertEQ(comp->getNextSectionRequest(i), 0);
    }

    const int cOctave = 2;

    comp->params[comp->CV_SELECT_OCTAVE_PARAM].value = float(cOctave);
    comp->inputs[comp->SELECT_CV_INPUT].channels = 4;
    comp->inputs[comp->SELECT_GATE_INPUT].channels = 4;

    for (int i = 0; i < 4; ++i) {
        comp->inputs[comp->SELECT_CV_INPUT].setVoltage(-2, i);
        comp->inputs[comp->SELECT_GATE_INPUT].setVoltage(0, i);
    }

    // set no gate in channel 0
    comp->inputs[comp->SELECT_CV_INPUT].setVoltage(-2, 0);
    comp->inputs[comp->SELECT_GATE_INPUT].setVoltage(0, 0);

    play(comp, rate, .2f);
    const int target = 2;       // want req for section 2 (zero based)

    float cvArray[4];

    cvArray[0] = PitchUtils::pitchToCV(cOctave, target);
    cvArray[1]= PitchUtils::pitchToCV(cOctave, target + 4);
    cvArray[2] = PitchUtils::pitchToCV(cOctave, target + 2 * 4);
    cvArray[3] = PitchUtils::pitchToCV(cOctave, target + 3 * 4);
   // const int row = sectionToSelect / MidiSong4::numTracks;
   // const int col = sectionToSelect % MidiSong4::numTracks;
    //printf("in test row=%d col=%d\n", row, col);

   // assert(comp->getSong()->getTrack(row, col));



    /// now play all 4 triggers
    for (int i = 0; i < 4; ++i) {
        comp->inputs[comp->SELECT_CV_INPUT].setVoltage(cvArray[i], i);
        comp->inputs[comp->SELECT_GATE_INPUT].setVoltage(10, i);
    }
    play(comp, rate, .3f);

    // now check that the correct section is requested

    for (int r = 0; r < MidiSong4::numTracks; ++r) {
        const int expectedRequest = 3;
        assertEQ(comp->getNextSectionRequest(r), expectedRequest);
    }
}


static void testLabels()
{
    auto x = Sq4::getPolyLabels();
    assert(x.size() == 16);
    x = Sq4::getCVFunctionLabels();
    assert(x.size() == 4);
}

void testSeqComposite4()
{
    test0();
    testPause();
    testPauseSwitchSectionStart();
    testLabels();
    testSelectSectionWithCV();
    testSelectSectionWithCVPoly();
}