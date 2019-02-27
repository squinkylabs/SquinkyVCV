
#include "AboveNoteGrid.h"
#include "MidiSequencer.h"
#include "UIPrefs.h"

AboveNoteGrid::AboveNoteGrid(const Vec& pos, const Vec& size, MidiSequencerPtr seq)
{
    this->box.pos = pos;
    box.size = size;
    sequencer = seq;

    editAttributeLabel = new Label();
    editAttributeLabel->box.pos = Vec(10, 10);
    editAttributeLabel->text = "";
    editAttributeLabel->color = UIPrefs::SELECTED_NOTE_COLOR;
    addChild(editAttributeLabel);
}

void AboveNoteGrid::setSequencer(MidiSequencerPtr seq)
{
    sequencer = seq;
}

void AboveNoteGrid::step()
{
    auto attr = sequencer->context->noteAttribute;
    if (firstTime || (curAttribute != attr)) {
        curAttribute = attr;
        switch (attr) {
            case MidiEditorContext::NoteAttribute::Pitch:
                editAttributeLabel->text = "Pitch";
                break;
            case MidiEditorContext::NoteAttribute::Duration:
                editAttributeLabel->text = "Duration";
                break;
            case MidiEditorContext::NoteAttribute::StartTime:
                editAttributeLabel->text = "Start Time";
                break;
            default:
                assert(false);
        }
    }
    firstTime = false;
}


// TODO: move to util
static void filledRect(NVGcontext *vg, NVGcolor color, float x, float y, float w, float h)
{
    nvgFillColor(vg, color);
    nvgBeginPath(vg);
    nvgRect(vg, x, y, w, h);
    nvgFill(vg);
}


#ifdef __V1
void AboveNoteGrid::draw(const DrawArgs &args)
{
    NVGcontext *vg = args.vg;
#else
void AboveNoteGrid::draw(NVGcontext *vg)
{
#endif

    filledRect(vg, UIPrefs::NOTE_EDIT_BACKGROUND, 0, 0, box.size.x, box.size.y);

#ifdef __V1
    OpaqueWidget::draw(args);
#else
    OpaqueWidget::draw(vg);
#endif
}