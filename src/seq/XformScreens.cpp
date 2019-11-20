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
    };
    int row = 0;
    addPitchInput(Vec(centerColumn, controlRow(row)), "Pitch inversion axis", keepInScaleCallback);

    row += 2;

    auto keysig = seq->context->settings()->getKeysig();
    addKeysigInput(Vec(centerColumn, controlRow(row)), keysig);
}

void XformInvert::execute()
{
    const auto keysig = getKeysig(1);
    saveKeysig(1);

    XformLambda xform;
    PitchInputWidget* widget = dynamic_cast<PitchInputWidget*>(inputControls[0]);
    assert(widget);
    const int octave = widget->absoluteOctaves();
    if (widget->isChromaticMode()) {
        int axisSemis = widget->absoluteSemis();
        const int axisTotal = axisSemis + 12 * octave;
        xform = Scale::makeInvertLambdaChromatic(axisTotal);
    } else {
        ScalePtr scale = Scale::getScale(keysig.second, keysig.first);
        const int axisDegreesPartial = widget->absoluteDegrees();
        const int axisDegrees = scale->octaveAndDegree(octave, axisDegreesPartial);
        xform = Scale::makeInvertLambdaDiatonic(axisDegrees, keysig.first, keysig.second);
    }

    ReplaceDataCommandPtr cmd = ReplaceDataCommand::makeFilterNoteCommand(
        "Invert", sequencer, xform);
    sequencer->undo->execute(sequencer, cmd);
}

XformTranspose::XformTranspose(
    const ::rack::math::Vec& pos,
    const ::rack::math::Vec& size,
    MidiSequencerPtr seq,
    std::function<void(bool)> dismisser) : InputScreen(pos, size, seq, "Transpose Pitch", dismisser)
{
    // row 0 = transpose amount
    // row 1 = keysig

    auto keepInScaleCallback = [this]() {
        // Now I would look at constrain state, and use to update
        // visibility of keysigs
        const bool constrain = inputControls[0]->getValue() > .5;
        inputControls[1]->enable(constrain);
        inputControls[2]->enable(constrain);
    };

    // Row 0,transpose amount
    int row = 0;
    addPitchOffsetInput(Vec(centerColumn, controlRow(row)), "Transpose Amount", keepInScaleCallback);
      
    // row 2, 3
    row += 2;      // above takes two rows

   // bool enableKeysig = false;
    auto keysig = seq->context->settings()->getKeysig();
    addKeysigInput(Vec(centerColumn, controlRow(row)), keysig);
    keepInScaleCallback();          // init the keysig visibility
}

void XformTranspose::execute()
{ 
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

        // TODO: replace this math with the helper
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
    std::function<void(bool)> dismisser) : InputScreen(pos, size, seq, "Reverse Pitch", dismisser)
{
}

void XformReversePitch::execute()
{
    ReplaceDataCommandPtr cmd = ReplaceDataCommand::makeReversePitchCommand(sequencer);
    sequencer->undo->execute(sequencer, cmd);
}

std::vector<std::string> ornaments = {
    "None", "Trill", "Arpeggio"  
};

XformChopNotes::XformChopNotes(
    const ::rack::math::Vec& pos,
    const ::rack::math::Vec& size,
    MidiSequencerPtr seq,
    std::function<void(bool)> dismisser) : InputScreen(pos, size, seq, "Chop Notes", dismisser)
{
    // row 0, num notes
    // control 0 :
    int row = 0;    
    addNumberChooserInt(Vec(centerColumn, controlRow(row)), "Notes", 2, 11);

    // row 1, ornaments
    // control 1, ornaments
    ++row;
    addChooser(Vec(centerColumn, controlRow(row)), 76, "Ornament", ornaments);

    // make this re-usable!!!
    auto keepInScaleCallback = [this]() {
        // Now I would look at constrain state, and use to update
        // visibility of keysigs
        const bool constrain = inputControls[2]->getValue() > .5;
        inputControls[3]->enable(constrain);
        inputControls[4]->enable(constrain);
    };


    // Row 2,3
    // control 2 is keep in scale
    ++row;
    addPitchOffsetInput(Vec(centerColumn, controlRow(row)), "Steps", keepInScaleCallback);

    // control 3, 4 keysig
    row += 2;
   // bool enableKeysig = false;
    auto keysig = seq->context->settings()->getKeysig();
    addKeysigInput(Vec(centerColumn, controlRow(row)), keysig);
    keepInScaleCallback();          // init the keysig visibility

}

void XformChopNotes::execute()
{
    PitchInputWidget* widget = dynamic_cast<PitchInputWidget*>(inputControls[2]);
    assert(widget);

    int chopSteps = 0;
    ScalePtr scale;

    const bool chromatic = widget->isChromaticMode();
    const int octave = widget->transposeOctaves();
    if (chromatic) {
        const int semitones = widget->transposeSemis();
        const int totalSemitones = semitones + 12 * octave;
        chopSteps = totalSemitones;
    } else {
        auto keysig = getKeysig(3);
        saveKeysig(3);
        scale = Scale::getScale(keysig.second, keysig.first);

        // TODO: replace this math with the helper
        const int scaleDegrees = widget->transposeDegrees() + octave * scale->degreesInScale();
        chopSteps = scaleDegrees;
    }

    // TODO: fix this offset
    const int numNotes = 2 + int( std::round(inputControls[0]->getValue()));
    ReplaceDataCommand::Ornament ornament =  ReplaceDataCommand::Ornament(
        std::round(inputControls[1]->getValue()));
    ReplaceDataCommandPtr cmd = ReplaceDataCommand::makeChopNoteCommand(
        sequencer, 
        numNotes,  
        ornament, 
        scale, 
        chopSteps);
    sequencer->undo->execute(sequencer, cmd);
}


XFormQuantizePitch::XFormQuantizePitch(
    const ::rack::math::Vec& pos,
    const ::rack::math::Vec& size,
    MidiSequencerPtr seq,
    std::function<void(bool)> dismisser) : InputScreen(pos, size, seq, "Chop Notes", dismisser)
{
    int row = 0;
    auto keysig = seq->context->settings()->getKeysig();
    addKeysigInput(Vec(centerColumn, controlRow(row)), keysig); 
}

void XFormQuantizePitch::execute()
{
    auto keysig = getKeysig(0);
    saveKeysig(0);
    //scale = Scale::getScale(keysig.second, keysig.first);
    XformLambda xform = Scale::makeQuantizePitchLambda(keysig.first, keysig.second);
    ReplaceDataCommandPtr cmd = ReplaceDataCommand::makeFilterNoteCommand(
        "Quantize Pitch", sequencer, xform);
    sequencer->undo->execute(sequencer, cmd);
}

//****************************** Make Triads 

std::vector<std::string> triads = {
    "Root Position", "First Inversion", "Second Inversion", "Auto"  
};


XFormMakeTriads::XFormMakeTriads(
    const ::rack::math::Vec& pos,
    const ::rack::math::Vec& size,
    MidiSequencerPtr seq,
    std::function<void(bool)> dismisser) : InputScreen(pos, size, seq, "Make Triads", dismisser)
{
    int row = 0;
    addChooser(Vec(centerColumn, controlRow(row)), 130, "Triad type", triads);
    ++row;
    auto keysig = seq->context->settings()->getKeysig();
    addKeysigInput(Vec(centerColumn, controlRow(row)), keysig); 
}

void XFormMakeTriads::execute()
{
    auto keysig = getKeysig(1);
    saveKeysig(1);

    ScalePtr scale = Scale::getScale(keysig.second, keysig.first);
    ReplaceDataCommand::TriadType triadType =  ReplaceDataCommand::TriadType(
        std::round(inputControls[0]->getValue()));
    ReplaceDataCommandPtr cmd = ReplaceDataCommand::makeMakeTriadsCommand(
        sequencer, 
        triadType,
        scale);
    sequencer->undo->execute(sequencer, cmd);
}

