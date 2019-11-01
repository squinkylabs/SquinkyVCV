#include "InputControls.h"
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
    row += 2;
    addKeysigInput(Vec(centerColumn, controlRow(row)));
}

void XformInvert::execute()
{
    WARN("Entering xform invert execute our selection has %d notes",  sequencer->selection->size());
    const float pitchAxisCV = getAbsPitchFromInput(0);
    const bool constrain = getValueBool(2);
    const auto keysig = getKeysig(3);

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
    int row = 0;
    addPitchOffsetInput(Vec(centerColumn, controlRow(row)), "Transpose Amount");

    ++row;
    addConstrainToScale(Vec(centerColumn, controlRow(row)));

    row += 2;
    addKeysigInput(Vec(centerColumn, controlRow(row)));
}

void XformTranspose::execute()
{
    WARN("Entering xform execute our selection has %d notes",  sequencer->selection->size());
    const float transpose =  getPitchOffsetAmount(0);
    const bool constrain = getValueBool(2);
    auto keysig = getKeysig(3);

    auto lambda = DiatonicUtils::makeTransposeLambda(
        transpose, constrain, keysig.first, keysig.second);

    ReplaceDataCommandPtr cmd = ReplaceDataCommand::makeFilterNoteCommand(
        "Transpose", sequencer, lambda);
    sequencer->undo->execute(sequencer, cmd);
}