
#include "InputControls.h"
#include "MidiSequencer.h"
#include "SqMidiEvent.h"
#include "ReplaceDataCommand.h"
#include "XformScreens.h"


//const NVGcolor TEXT_COLOR = nvgRGB(0xc0, 0xc0, 0xc0);

using Widget = ::rack::widget::Widget;
using Vec = ::rack::math::Vec;
using Label = ::rack::ui::Label;

//**************************** Invert *********************************
XformInvert::XformInvert(
    const ::rack::math::Vec& pos,
    const ::rack::math::Vec& size,
    MidiSequencerPtr seq,
    std::function<void()> dismisser) : InputScreen(pos, size, seq, dismisser)
{
    float x = 100;
    float y = 30;
    addPitchInput(Vec(x, y), "Axis");

    auto check = new CheckBox();
    check->box.pos = Vec(10, 10);
    check->box.size = Vec(17, 17);
    this->addChild(check);
}

void XformInvert::execute()
{
    WARN("Entering xform execute our selection has %d notes",  sequencer->selection->size());
    WARN("execute");
    float pitchAxis = getAbsPitchFromInput(0);
    auto lambda = [pitchAxis](MidiEventPtr p) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(p);
        if (note) {
            WARN("in lambda");
            note->pitchCV = pitchAxis - note->pitchCV;
        }
    };


    WARN("now we need to invert those notes!");
    ReplaceDataCommandPtr cmd = ReplaceDataCommand::makeFilterNoteCommand(
        "Invert", sequencer, lambda);
    sequencer->undo->execute(sequencer, cmd);

    fflush(stdout);
}