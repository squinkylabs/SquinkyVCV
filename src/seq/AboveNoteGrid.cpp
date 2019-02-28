#include "AboveNoteGrid.h"
#include "MidiSequencer.h"
#include "TimeUtils.h"
#include "UIPrefs.h"

#include <sstream>

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

    createTimeLabels();
}

void AboveNoteGrid::setSequencer(MidiSequencerPtr seq)
{
    sequencer = seq;
}

void AboveNoteGrid::step()
{
    if (!sequencer) {
        return;
    }
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

void AboveNoteGrid::createTimeLabels()
{
    
}

// TODO: move to util
static void filledRect(NVGcontext *vg, NVGcolor color, float x, float y, float w, float h)
{
    nvgFillColor(vg, color);
    nvgBeginPath(vg);
    nvgRect(vg, x, y, w, h);
    nvgFill(vg);
}




void AboveNoteGrid::drawTimeLabels(NVGcontext *vg)
{
    #if 0
    int firstBar = 1 + TimeUtils::timeToBar(sequencer->context->startTime());
    if (firstBar != curFirstBar) {
        curFirstBar = firstBar;
        std::stringstream str;
        str << "First Bar: " << curFirstBar << " Last Bar: " << curFirstBar + 1;
        barRangeLabel->text = str.str();
    }
    #endif
}


#ifdef __V1
void AboveNoteGrid::draw(const DrawArgs &args)
{
    NVGcontext *vg = args.vg;
#else
void AboveNoteGrid::draw(NVGcontext *vg)
{
#endif


    if (!this->sequencer) {
        return;
    }

    filledRect(vg, UIPrefs::NOTE_EDIT_BACKGROUND, 0, 0, box.size.x, box.size.y);
    drawTimeLabels(vg);

#ifdef __V1
    OpaqueWidget::draw(args);
#else
    OpaqueWidget::draw(vg);
#endif
}