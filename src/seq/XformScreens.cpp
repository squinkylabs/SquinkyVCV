#include "InputControls.h"
#include "ISeqSettings.h"
#include "MidiSequencer.h"
#include "PitchInputWidget.h"
#include "Scale.h"
#include "SqMidiEvent.h"
#include "ReplaceDataCommand.h"
#include "XformScreens.h"

using Widget = ::rack::widget::Widget;
using Vec = ::rack::math::Vec;
using Label = ::rack::ui::Label;

//**************************** Invert *********************************

XformInvert::XformInvert(
    const ::rack::math::Vec& pos,
    const ::rack::math::Vec& size,
    MidiSequencerPtr seq,
    std::function<void(bool)> dismisser) : InputScreen(pos, size, seq, "Invert Pitch", dismisser)
{

    auto keepInScaleCallback = [this]() {
        // Now I would look at constrain state, and use to update
        // visibility of keysigs
        const bool constrain = inputControls[0]->getValue() > .5;
        inputControls[1]->enable(constrain);
        inputControls[2]->enable(constrain);
        WARN("in xpose callback x set constrain %d", constrain);
    };
    int row = 0;
    addPitchInput(Vec(centerColumn, controlRow(row)), "Pitch inversion axis", keepInScaleCallback);

    row += 2;

    auto keysig = seq->context->settings()->getKeysig();
    addKeysigInput(Vec(centerColumn, controlRow(row)), keysig);
}

void XformInvert::execute()
{
    WARN("Entering xform invert execute our selection has %d notes",  sequencer->selection->size());

    const auto keysig = getKeysig(1);
    saveKeysig(1);

    PitchInputWidget* widget = dynamic_cast<PitchInputWidget*>(inputControls[0]);
    assert(widget);
    if (widget->isChromaticMode()) {
        int semisToTranspose = widget->transposeSemis();
        WARN("chromaitc nimp");
    } else {
        int scaleDegreesToTranspose = widget->transposeDegrees();
        WARN("scale relative nimp");
    }

#if 0
    const int pitchAxisSemitones = PitchUtils::cvToSemitone(pitchAxisCV);
    DEBUG("pitch axis cv was %.2f, semi = %d", pitchAxisCV, pitchAxisSemitones);

    auto lambda = DiatonicUtils::makeInvertLambda(
        pitchAxisSemitones, constrain, keysig.first, keysig.second);
    ReplaceDataCommandPtr cmd = ReplaceDataCommand::makeFilterNoteCommand(
        "Invert", sequencer, lambda);
    sequencer->undo->execute(sequencer, cmd);
#endif
}

XformTranspose::XformTranspose(
    const ::rack::math::Vec& pos,
    const ::rack::math::Vec& size,
    MidiSequencerPtr seq,
    std::function<void(bool)> dismisser) : InputScreen(pos, size, seq, "Transpose Pitch", dismisser)
{
    DEBUG("\n\n************* in ctol of Xftran");

    // row 0 = transpose amount
    // row 1 = keysig

    auto keepInScaleCallback = [this]() {
        // Now I would look at constrain state, and use to update
        // visibility of keysigs
        const bool constrain = inputControls[0]->getValue() > .5;
        inputControls[1]->enable(constrain);
        inputControls[2]->enable(constrain);
        WARN("in xpose callback x set constrain %d", constrain);
    };

    // Row 0,transpose amount
    int row = 0;
    addPitchOffsetInput(Vec(centerColumn, controlRow(row)), "Transpose Amount", keepInScaleCallback);
    DEBUG("after add pitch offset there are %d controls", inputControls.size());
      
    // row 2, 3
    row += 2;      // above takes two rows

    bool enableKeysig = false;
    auto keysig = seq->context->settings()->getKeysig();
    DEBUG("in transpos ctor, keysig = %d,%d", keysig.first, keysig.second);
    addKeysigInput(Vec(centerColumn, controlRow(row)), keysig);
    DEBUG("enable keysig = %d", enableKeysig);

    keepInScaleCallback();          // init the keysig visibility

   // inputControls[1]->enable(enableKeysig);
  //  inputControls[2]->enable(enableKeysig);

     DEBUG("end of xform ctor there are %d controls", inputControls.size());
}

void XformTranspose::execute()
{
    WARN("Entering xform execute our selection has %d notes",  sequencer->selection->size());
    DEBUG("there are %d controls", inputControls.size());
 
    XformLambda xform;
    PitchInputWidget* widget = dynamic_cast<PitchInputWidget*>(inputControls[0]);
    assert(widget);
    const bool chromatic = widget->isChromaticMode();
    const int octave = widget->transposeOctaves();
    if (chromatic) {
        const int semitones = widget->transposeSemis();
        xform = Scale::makeTransposeLambdaChromatic(semitones + 12 * octave);
    } else {
        auto keysig = getKeysig(1);
        saveKeysig(1);
        ScalePtr scale = Scale::getScale(keysig.second, keysig.first);

        const int scaleDegrees = widget->transposeDegrees() + octave * scale->degreesInScale();
        xform = Scale::makeTransposeLambdaScale(scaleDegrees, keysig.first, keysig.second);
    }

    ReplaceDataCommandPtr cmd = ReplaceDataCommand::makeFilterNoteCommand(
        "Transpose", sequencer, xform);
    sequencer->undo->execute(sequencer, cmd);
}


XformReversePitch::XformReversePitch(
    const ::rack::math::Vec& pos,
    const ::rack::math::Vec& size,
    MidiSequencerPtr seq,
    std::function<void(bool)> dismisser) : InputScreen(pos, size, seq, "Transpose Pitch", dismisser)
{
}


void XformReversePitch::execute()
{
    ReplaceDataCommandPtr cmd = ReplaceDataCommand::makeReversePitchCommand(sequencer);
    sequencer->undo->execute(sequencer, cmd);
}


XformChopNotes::XformChopNotes(
    const ::rack::math::Vec& pos,
    const ::rack::math::Vec& size,
    MidiSequencerPtr seq,
    std::function<void(bool)> dismisser) : InputScreen(pos, size, seq, "Chop Notes", dismisser)
{
    int row = 0;    
    addNumberChooserInt(Vec(centerColumn, controlRow(row)), "Notes", 2, 11);
}


void XformChopNotes::execute()
{
    // TODO: fix this offset
    const int numNotes = 2 + int( std::round(inputControls[0]->getValue()));
    DEBUG("num notes from control = %d\n", numNotes);
    ReplaceDataCommandPtr cmd = ReplaceDataCommand::makeChopNoteCommand(sequencer, numNotes);
    sequencer->undo->execute(sequencer, cmd);
}
