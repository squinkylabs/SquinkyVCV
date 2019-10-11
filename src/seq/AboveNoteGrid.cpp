#include "AboveNoteGrid.h"
#include "MidiSequencer.h"
#include "NoteScreenScale.h"
#include "TimeUtils.h"
#include "UIPrefs.h"

#include <sstream>

const float row1 = 10;
const float row2 = 40;

AboveNoteGrid::AboveNoteGrid(const Vec& pos, const Vec& size, MidiSequencerPtr seq)
{
    this->box.pos = pos;
    box.size = size;
    sequencer = seq;

    editAttributeLabel = new Label();
    editAttributeLabel->box.pos = Vec(10, row1);
    editAttributeLabel->text = "";
    editAttributeLabel->color = UIPrefs::SELECTED_NOTE_COLOR;
    addChild(editAttributeLabel);

    curLoop = std::make_shared<SubrangeLoop>();
}


void AboveNoteGrid::setSequencer(MidiSequencerPtr seq)
{
    sequencer = seq;
}

void AboveNoteGrid::songUpdated()
{
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

    updateTimeLabels();
    updateCursorLabels();
}

void AboveNoteGrid::createTimeLabels()
{
    auto scaler = sequencer->context->getScaler();
    assert(scaler);
    //assume two bars, quarter note grid
    float totalDuration = TimeUtils::bar2time(2);
    float deltaDuration = 1.f;
    for (float time = 0; time < totalDuration; time += deltaDuration) {
        // need delta.
        const float x = scaler->midiTimeToX(time) - 9;
        Label* label = new Label();
        label->box.pos = Vec(x, row2);
        label->text = "";
        label->color = UIPrefs::TIME_LABEL_COLOR;
        label->fontSize = UIPrefs::timeLabelFontSize;
        addChild(label);
        timeLabels.push_back(label);
    }

    cursorTimeLabel = new Label();
    cursorTimeLabel->box.pos = Vec(200, row1);
    cursorTimeLabel->text = "";
    cursorTimeLabel->color = UIPrefs::STATUS_LABEL_COLOR;
    addChild(cursorTimeLabel);

    cursorPitchLabel = new Label();
    cursorPitchLabel->box.pos = Vec(350, row1);
    cursorPitchLabel->text = "";
    cursorPitchLabel->color = UIPrefs::STATUS_LABEL_COLOR;
    addChild(cursorPitchLabel);

    loopLabel = new Label();
    loopLabel->box.pos = Vec(100, row1);
    loopLabel->text = "";
    loopLabel->color =  UIPrefs::STATUS_LABEL_COLOR;
    addChild(loopLabel);
}

// TODO: move to util
static void filledRect(NVGcontext *vg, NVGcolor color, float x, float y, float w, float h)
{
    nvgFillColor(vg, color);
    nvgBeginPath(vg);
    nvgRect(vg, x, y, w, h);
    nvgFill(vg);
}

void AboveNoteGrid::updateCursorLabels()
{
    if (curCursorTime != sequencer->context->cursorTime()) {
        curCursorTime = sequencer->context->cursorTime();
        cursorTimeLabel->text = TimeUtils::time2str(curCursorTime);
    }
    if (curCursorPitch != sequencer->context->cursorPitch()) {
        curCursorPitch = sequencer->context->cursorPitch();
        cursorPitchLabel->text = PitchUtils::pitch2str(curCursorPitch);
    }

    if (*curLoop != sequencer->song->getSubrangeLoop()) {
        *curLoop = sequencer->song->getSubrangeLoop();
        if (!curLoop->enabled) {
            loopLabel->text.erase();    
        } else {
            std::stringstream str;
            str << "L ";
            str << 1 + TimeUtils::time2bar(curLoop->startTime);
            str << '-';
            str << 1 + TimeUtils::time2bar(curLoop->endTime);
            loopLabel->text = str.str();
        }
    }
}

void AboveNoteGrid::updateTimeLabels()
{
    if (timeLabels.empty()) {
        createTimeLabels();
    }

    int firstBar = 1 + TimeUtils::time2bar(sequencer->context->startTime());
    if (firstBar == curFirstBar) {
        return;
    }

    curFirstBar = firstBar;
    auto scaler = sequencer->context->getScaler();
    assert(scaler);
    //assume two bars, quarter note spacing of labels
    float totalDuration = TimeUtils::bar2time(2);
    float deltaDuration = 1.f;
    int i = 0;
    for (float relTime = 0; relTime < totalDuration; relTime += deltaDuration) {
        const float time = relTime + sequencer->context->startTime();
        const bool isBar = !(i % 4);
        const int numDigitsDisplayed = isBar ? 1 : 2;
        std::string s = TimeUtils::time2str(time, numDigitsDisplayed);
       // printf("in label update, i = %d, size = %d\n", i, timeLabels.size());
        assert((int) timeLabels.size() > i);
        timeLabels[i]->text = s;
        ++i;
    }
}


void AboveNoteGrid::draw(const DrawArgs &args)
{
    NVGcontext *vg = args.vg;

    if (!this->sequencer) {
        return;
    }

    filledRect(vg, UIPrefs::NOTE_EDIT_BACKGROUND, 0, 0, box.size.x, box.size.y);
    OpaqueWidget::draw(args);
}