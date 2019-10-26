
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
    std::function<void(bool)> dismisser) : InputScreen(pos, size, seq, dismisser)
{
    int row = 0;
    addPitchInput(Vec(centerColumn, controlRow(row)), "Pitch inversion axis");

    ++row;
    addConstrainToScale(Vec(centerColumn, controlRow(row)));
}

void XformInvert::execute()
{
    WARN("Entering xform execute our selection has %d notes",  sequencer->selection->size());
    float pitchAxis = getAbsPitchFromInput(0);
    auto lambda = [pitchAxis](MidiEventPtr p) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(p);
        if (note) {
            WARN("in lambda");
            note->pitchCV = pitchAxis - note->pitchCV;
        }
    };

    ReplaceDataCommandPtr cmd = ReplaceDataCommand::makeFilterNoteCommand(
        "Invert", sequencer, lambda);
    sequencer->undo->execute(sequencer, cmd);
}
