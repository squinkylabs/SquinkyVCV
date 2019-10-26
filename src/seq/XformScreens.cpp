
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
    std::function<void(bool)> dismisser) : InputScreen(pos, size, seq, dismisser)
{
    float x = 170;
    float y = 40;
    addPitchInput(Vec(x, y), "Pitch inversion axis");

    y += 30;

    auto check = new CheckBox();
    check->box.pos = Vec(x, y);
    check->box.size = Vec(17, 17);
    this->addChild(check);

// rationalize this
    const NVGcolor TEXT_COLOR = nvgRGB(0xc0, 0xc0, 0xc0);
    x = 0;
    auto l = addLabel(Vec(x, y), "Constrain to scale", TEXT_COLOR );

    // move these magic numbers.
    l->box.size.x = 170 - 10;
    DEBUG("label x = %f, width = %f", l->box.pos.x, l->box.size.x);
    l->alignment = Label::RIGHT_ALIGNMENT;
  
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

    ReplaceDataCommandPtr cmd = ReplaceDataCommand::makeFilterNoteCommand(
        "Invert", sequencer, lambda);
    sequencer->undo->execute(sequencer, cmd);
}
