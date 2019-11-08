#include "InputControls.h"
#include "ISeqSettings.h"
#include "MidiSequencer.h"
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
    int row = 0;
    addPitchInput(Vec(centerColumn, controlRow(row)), "Pitch inversion axis");

    ++row;
    addConstrainToScale(Vec(centerColumn, controlRow(row)));
    inputControls[row]->setCallback( []() {
        WARN("in unvert callback x[");
    });

    row += 2;

  

    auto keysig = seq->context->settings()->getKeysig();
    addKeysigInput(Vec(centerColumn, controlRow(row)), keysig);
   
}

void XformInvert::execute()
{
    WARN("Entering xform invert execute our selection has %d notes",  sequencer->selection->size());
    const float pitchAxisCV = getAbsPitchFromInput(0);
    const bool constrain = getValueBool(2);
    const auto keysig = getKeysig(3);

    saveKeysig(3);

    const int pitchAxisSemitones = PitchUtils::cvToSemitone(pitchAxisCV);
    DEBUG("pitch axis cv was %.2f, semi = %d", pitchAxisCV, pitchAxisSemitones);
    
    auto lambda = DiatonicUtils::makeInvertLambda(
        pitchAxisSemitones, constrain, keysig.first, keysig.second);
    ReplaceDataCommandPtr cmd = ReplaceDataCommand::makeFilterNoteCommand(
        "Invert", sequencer, lambda);
    sequencer->undo->execute(sequencer, cmd);
}

XformTranspose::XformTranspose(
    const ::rack::math::Vec& pos,
    const ::rack::math::Vec& size,
    MidiSequencerPtr seq,
    std::function<void(bool)> dismisser) : InputScreen(pos, size, seq, "Transpose Pitch", dismisser)
{
    DEBUG("\n\n************* in ctol of Xftran");

    // Row 0,1 transpose amount
    int row = 0;
    addPitchOffsetInput(Vec(centerColumn, controlRow(row)), "Transpose Amount");
    
    
    row += 2;

    // row 2: constrain
    addConstrainToScale(Vec(centerColumn, controlRow(row)));
    DEBUG("set callback on row %d", row);
    inputControls[row]->setCallback( [this, row]() {
        // Now I would look at constrain state, and use to update
        // visibility of keysigs
        const bool constrain = inputControls[row]->getValue() > .5;
        inputControls[3]->enable(constrain);
        inputControls[4]->enable(constrain);
        WARN("in xpose callback x set constrain %d", constrain);
    });
    DEBUG("just added callback to control\n");

  
    // row 3, 4
    ++row;
    bool enableKeysig = false;
    auto keysig = seq->context->settings()->getKeysig();
    DEBUG("in transpos ctor, keysig = %d,%d", keysig.first, keysig.second);
    addKeysigInput(Vec(centerColumn, controlRow(row)), keysig);
    DEBUG("enable keysig = %d", enableKeysig);
    inputControls[row]->enable(enableKeysig);
    inputControls[row+1]->enable(enableKeysig);
}

void XformTranspose::execute()
{
    WARN("Entering xform execute our selection has %d notes",  sequencer->selection->size());
    const float transpose =  getPitchOffsetAmount(0);
    const bool constrain = getValueBool(2);
    auto keysig = getKeysig(3);

    saveKeysig(3);

    auto lambda = DiatonicUtils::makeTransposeLambda(
        transpose, constrain, keysig.first, keysig.second);

    ReplaceDataCommandPtr cmd = ReplaceDataCommand::makeFilterNoteCommand(
        "Transpose", sequencer, lambda);
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
