
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
    WARN("Entering xform execute our selection has %d notes",  sequencer->selection->size());
    float pitchAxis = getAbsPitchFromInput(0);
    #if 1
    auto lambda = [pitchAxis](MidiEventPtr p) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(p);
        if (note) {
            WARN("in lambda");
            note->pitchCV = pitchAxis - note->pitchCV;
        }
    };
    #else
    //auto lambda = DiatonicUtils::makeInvertLambda(axis, constrain, root, mode);
    #endif

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
    const float transpose =  getTransposeAmount(0);
    const bool constrain = getValueBool(2);
    auto keysig = getKeysig(3);

#if 0
    auto lambda = [transpose](MidiEventPtr p) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(p);
        if (note) {
            WARN("in lambda");
            note->pitchCV = transpose + note->pitchCV;
        }
    };
#endif

    auto lambda = DiatonicUtils::makeInvertLambda(
        transpose, constrain, keysig.first, keysig.second);

    ReplaceDataCommandPtr cmd = ReplaceDataCommand::makeFilterNoteCommand(
        "Invert", sequencer, lambda);
    sequencer->undo->execute(sequencer, cmd);
}