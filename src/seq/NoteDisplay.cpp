#include "../Squinky.hpp"

#ifdef _SEQ
#include "WidgetComposite.h"
#include "Seq.h"

#ifdef __V1
#include "widget/Widget.hpp"
#else
#include "widgets.hpp"
#include "util/math.hpp"
#endif

#include "nanovg.h"
#include "window.hpp"
//#include "MidiEditorContext.h"
#include "MidiSequencer.h"
#include <GLFW/glfw3.h>
#include "UIPrefs.h"
#include "MidiKeyboardHandler.h"
#include "NoteScreenScale.h"
#include "PitchUtils.h"
#include "../ctrl/SqHelper.h"
#include "TimeUtils.h"
#include "NoteDisplay.h"

NoteDisplay::NoteDisplay(const Vec& pos, const Vec& size, MidiSequencerPtr seq)
{
    this->box.pos = pos;
    box.size = size;
    sequencer = seq;

    if (sequencer) {
        initEditContext();

        auto scaler2 = sequencer->context->getScaler();
        assert(scaler2);
    }
  
    focusLabel = new Label();
    focusLabel->box.pos = Vec(40, 40);
    focusLabel->text = "";
    focusLabel->color = SqHelper::COLOR_WHITE;
    addChild(focusLabel);
    updateFocus(false);
}

void NoteDisplay::setSequencer(MidiSequencerPtr seq)
{
    assert(seq.get());
    sequencer = seq;
    sequencer->assertValid();
    initEditContext();
}

void NoteDisplay::initEditContext()
{
    // hard code view range (for now?)
    sequencer->context->setStartTime(0);
    sequencer->context->setEndTime(8);
    sequencer->context->setPitchLow(PitchUtils::pitchToCV(3, 0));
    sequencer->context->setPitchHi(PitchUtils::pitchToCV(5, 0));
    sequencer->editor->updateSelectionForCursor();

// set scaler once context has a valid range
    auto scaler = std::make_shared<NoteScreenScale>(
        box.size.x,
        box.size.y,
        UIPrefs::hMarginsNoteEdit,
        UIPrefs::topMarginNoteEdit);
    sequencer->context->setScaler(scaler);
    assert(scaler);
}

// TODO: get rid of this
void NoteDisplay::step()
{
    if (!sequencer) {
        return;
    }
}

void NoteDisplay::drawNotes(NVGcontext *vg)
{
    MidiEditorContext::iterator_pair it = sequencer->context->getEvents();
    auto scaler = sequencer->context->getScaler();
    assert(scaler);
    const int noteHeight = scaler->noteHeight();
    for (; it.first != it.second; ++it.first) {
        auto temp = *(it.first);
        MidiEventPtr evn = temp.second;
        MidiNoteEventPtr ev = safe_cast<MidiNoteEvent>(evn);

        const float x = scaler->midiTimeToX(*ev);
        const float y = scaler->midiPitchToY(*ev);
        const float width = scaler->midiTimeTodX(ev->duration);

        const bool selected = sequencer->selection->isSelected(ev);
        filledRect(
            vg,
            selected ? UIPrefs::SELECTED_NOTE_COLOR : UIPrefs::NOTE_COLOR,
            x, y, width, noteHeight);
    }
}

void NoteDisplay::drawGrid(NVGcontext *vg)
{
    // float z = APP->scene->zoomWidget->zoom;
    //  printf("zoom is %f\n", z); fflush(stdout);

    auto scaler = sequencer->context->getScaler();
    assert(scaler);
    //assume two bars, quarter note grid
    float totalDuration = TimeUtils::bar2time(2);
    float deltaDuration = 1.f;
    for (float relTime = 0; relTime <= totalDuration; relTime += deltaDuration) {
        const float time =  relTime + sequencer->context->startTime();
        const float x = scaler->midiTimeToX(time);
        const float y = UIPrefs::topMarginNoteEdit;
        float width = 2;
        float height = this->box.size.y - y;

        const bool isBar = (relTime == 0) ||
            (relTime == TimeUtils::bar2time(1)) ||
            (relTime == TimeUtils::bar2time(2));

        filledRect(
            vg,
            isBar ? UIPrefs::GRID_BAR_COLOR : UIPrefs::GRID_COLOR,
            x, y, width, height);
    }
}

void NoteDisplay::drawCursor(NVGcontext *vg)
{
    cursorFrameCount--;
    if (cursorFrameCount < 0) {
        cursorFrameCount = 10;
        cursorState = !cursorState;
    }

    if (true) {
        auto color = cursorState ?
            nvgRGB(0xff, 0xff, 0xff) :
            nvgRGB(0, 0, 0);

        auto scaler = sequencer->context->getScaler();
        assert(scaler);

        const float x = scaler->midiTimeToX(sequencer->context->cursorTime());
        const float y = scaler->midiCvToY(sequencer->context->cursorPitch()) +
            scaler->noteHeight() / 2.f;
        filledRect(vg, color, x, y, 10, 3);
    }
}

#ifdef __V1
void NoteDisplay::draw(const Widget::DrawArgs &args)
{
    NVGcontext *vg = args.vg;
#else
void NoteDisplay::draw(NVGcontext *vg)
{
#endif 

    if (!this->sequencer) {
        return;
    }
    drawBackground(vg);
    drawGrid(vg);
    drawNotes(vg);
    drawCursor(vg);
#ifdef __V1
    OpaqueWidget::draw(args);
#else
    OpaqueWidget::draw(vg);
#endif
}

void NoteDisplay::drawBackground(NVGcontext *vg)
{
    auto scaler = sequencer->context->getScaler();
    filledRect(vg, UIPrefs::NOTE_EDIT_BACKGROUND, 0, 0, box.size.x, box.size.y);
    assert(scaler);
    const int noteHeight = scaler->noteHeight();
    const float width = box.size.x;
    for (float cv = sequencer->context->pitchLow();
        cv <= sequencer->context->pitchHi();
        cv += PitchUtils::semitone) {

        const float y = scaler->midiCvToY(cv);
        
        bool accidental = PitchUtils::isAccidental(cv);
        if (accidental) {
            filledRect(
                vg,
                UIPrefs::NOTE_EDIT_ACCIDENTAL_BACKGROUND,
                0, y, width, noteHeight);
        }
    }

    for (float cv = sequencer->context->pitchLow();
        cv <= sequencer->context->pitchHi();
        cv += PitchUtils::semitone) {

        float y = scaler->midiCvToY(cv) + scaler->noteHeight();
        const bool isC = PitchUtils::isC(cv);
        if (y > (box.size.y - .5)) {
            y = y - 2;  // make sure  bottom line draws. Should really
                        // re-design the visuals here
        }
        if (isC) {
         //   const float y = scaler->midiCvToY(cv);
            filledRect(
                vg,
                UIPrefs::GRID_CLINE_COLOR,
                0, y, width, 1);
        }
    }
}

void NoteDisplay::strokedRect(NVGcontext *vg, NVGcolor color, float x, float y, float w, float h)
{
    nvgStrokeColor(vg, color);
    nvgBeginPath(vg);
    nvgRect(vg, x, y, w, h);
    nvgStroke(vg);
}

void NoteDisplay::filledRect(NVGcontext *vg, NVGcolor color, float x, float y, float w, float h)
{
    nvgFillColor(vg, color);
    nvgBeginPath(vg);
    nvgRect(vg, x, y, w, h);
    nvgFill(vg);
}

/******************** All V1 keyboard handling here *******************
 * 
 * New idea:
 *      let's do all keyboard with hover key.
 *      if we consume the key, then grab focus.
 */
 

#ifdef __V1

void NoteDisplay::onDoubleClick(const widget::DoubleClickEvent &e)
{
   // printf("got double click"); fflush(stdout);
    OpaqueWidget::onDoubleClick(e);
}

void NoteDisplay::onButton(const ButtonEvent &e)
{
    // printf("got on button pos: %.2f, %.2f\n", e.pos.x, e.pos.y); fflush(stdout);
#if 0
struct ButtonEvent : Event, PositionEvent {
	/** GLFW_MOUSE_BUTTON_LEFT, GLFW_MOUSE_BUTTON_RIGHT, GLFW_MOUSE_BUTTON_MIDDLE, etc. */
	int button;
	/** GLFW_PRESS or GLFW_RELEASE */
	int action;
	/** GLFW_MOD_* */
	int mods;
};
#endif
    OpaqueWidget::onButton(e);
}

void NoteDisplay::onHoverKey(const HoverKeyEvent &e)
{
    bool handle = false;
    bool repeat = false;
    switch (e.action) {
        case GLFW_REPEAT:
            handle = false;
            repeat = true;
            break;
        case GLFW_PRESS:
            handle = true;
            repeat = false;
            break;
    }

    if (repeat) {
        handle = MidiKeyboardHandler::doRepeat(e.key);
    }

    bool handled = false;
    if (handle) {
        handled = MidiKeyboardHandler::handle(sequencer, e.key, e.mods);
        if (handled) {
            APP->event->setSelected(this);
           //updateFocus(true);
            e.consume(this);
        }
    }
    if (!handled) {
        OpaqueWidget::onHoverKey(e);
    }
}
void NoteDisplay::onSelect(const SelectEvent &e)
{
    updateFocus(true);
    e.consume(this);
}

void NoteDisplay::onDeselect(const DeselectEvent &e)
{
    updateFocus(false);
    e.consume(this);
}

#endif

//**************** All V0.6 keyboard handling here ***********

#ifndef __V1

void NoteDisplay::onFocus(EventFocus &e)
{
    updateFocus(true);
    e.consumed = true;
}

void NoteDisplay::onDefocus(EventDefocus &e)
{
    updateFocus(false);
    e.consumed = true;
}

void NoteDisplay::onKey(EventKey &e)
{
    const unsigned key = e.key;
    unsigned mods = 0;
    if (rack::windowIsShiftPressed()) {
        mods |= GLFW_MOD_SHIFT;
    }
    if (windowIsModPressed()) {
        mods |= GLFW_MOD_CONTROL;
    }

    bool handled = MidiKeyboardHandler::handle(sequencer, key, mods);
    if (!handled) {
        OpaqueWidget::onKey(e);
    } else {
        e.consumed = true;
    }
}

#endif
#endif
