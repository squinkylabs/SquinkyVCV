#include "../Squinky.hpp"

#ifdef _SEQ
#include "WidgetComposite.h"
#include "Seq.h"

#ifdef __V1x
#include "widget/Widget.hpp"
#else
#include "widgets.hpp"
#include "util/math.hpp"
#endif

#include "nanovg.h"
#include "window.hpp"
#include "MidiSequencer.h"
#include <GLFW/glfw3.h>
#include "UIPrefs.h"
#include "MidiKeyboardHandler.h"
#include "MouseManager.h"
#include "NoteScreenScale.h"
#include "PitchUtils.h"
#include "../ctrl/SqHelper.h"
#include "TimeUtils.h"
#include "NoteDisplay.h"
#include "SeqSettings.h"
#include "SqGfx.h"

NoteDisplay::NoteDisplay(
    const Vec& pos, 
    const Vec& size, 
    MidiSequencerPtr seq,
    rack::engine::Module* mod)
{
    this->box.pos = pos;
    box.size = size;
    sequencer = seq;
    mouseManager = std::make_shared<MouseManager>(sequencer);
  //  seqSettings = std::make_shared<SeqSettings>(mod);

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

    // re-associate seq and mouse manager
    mouseManager = std::make_shared<MouseManager>(sequencer);
}

void NoteDisplay::initEditContext()
{
    // hard code view range (for now?)
    sequencer->context->setStartTime(0);
    sequencer->context->setEndTime(8);
    sequencer->context->setPitchLow(PitchUtils::pitchToCV(3, 0));
    sequencer->context->setPitchHi(PitchUtils::pitchToCV(6, 0));        // was originally 5, for 2 octaves
    sequencer->editor->updateSelectionForCursor(false);

// set scaler once context has a valid range
    auto scaler = std::make_shared<NoteScreenScale>(
        box.size.x,
        box.size.y,
        UIPrefs::hMarginsNoteEdit,
        UIPrefs::topMarginNoteEdit);
    sequencer->context->setScaler(scaler);
    assert(scaler);
}

// TODO: get rid of this (dont remember why this is here)
void NoteDisplay::step()
{
    if (!sequencer) {
        return;
    }
    OpaqueWidget::step();
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
        if (!selected || !mouseManager->willDrawSelection()) {
            SqGfx::filledRect(
                vg,
                selected ? UIPrefs::SELECTED_NOTE_COLOR : UIPrefs::NOTE_COLOR,
                x, y, width, noteHeight);
        }
    }
}

void NoteDisplay::drawGrid(NVGcontext *vg)
{
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

        SqGfx::filledRect(
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
        SqGfx::filledRect(vg, color, x, y, 10, 3);
    }
}

#ifdef __V1x
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

    // let's clip everything to our window
    nvgScissor(vg, 0, 0, this->box.size.x, this->box.size.y);
    drawBackground(vg);
    drawGrid(vg);
    drawNotes(vg);
    
    // if we are dragging, will have something to draw
    mouseManager->draw(vg);  
    drawCursor(vg);   
#ifdef __V1x
    OpaqueWidget::draw(args);
#else
    OpaqueWidget::draw(vg);
#endif
}

void NoteDisplay::drawBackground(NVGcontext *vg)
{
    auto scaler = sequencer->context->getScaler();
    SqGfx::filledRect(vg, UIPrefs::NOTE_EDIT_BACKGROUND, 0, 0, box.size.x, box.size.y);
    assert(scaler);
    const int noteHeight = scaler->noteHeight();
    const float width = box.size.x;
    for (float cv = sequencer->context->pitchLow();
        cv <= sequencer->context->pitchHigh();
        cv += PitchUtils::semitone) {

        const float y = scaler->midiCvToY(cv);
        
        bool accidental = PitchUtils::isAccidental(cv);
        if (accidental) {
            SqGfx::filledRect(
                vg,
                UIPrefs::NOTE_EDIT_ACCIDENTAL_BACKGROUND,
                0, y, width, noteHeight);
        }
    }

    for (float cv = sequencer->context->pitchLow();
        cv <= sequencer->context->pitchHigh();
        cv += PitchUtils::semitone) {

        float y = scaler->midiCvToY(cv) + scaler->noteHeight();
        const bool isC = PitchUtils::isC(cv);
        if (y > (box.size.y - .5)) {
            y = y - 2;  // make sure  bottom line draws. Should really
                        // re-design the visuals here
        }
        if (isC) {
         //   const float y = scaler->midiCvToY(cv);
            SqGfx::filledRect(
                vg,
                UIPrefs::GRID_CLINE_COLOR,
                0, y, width, 1);
        }
    }
}

/******************** All V1 keyboard handling here *******************
 * 
 */
 
#ifdef __V1x

void NoteDisplay::onDoubleClick(const event::DoubleClick &e)
{
   // printf("got double click"); fflush(stdout);
   
    bool handled = mouseManager->onDoubleClick();
    if (handled) {
        e.consume(this);
    } else {
        OpaqueWidget::onDoubleClick(e);
    }
}

 void NoteDisplay::onDragDrop(const event::DragDrop &e) 
 {
     //printf("on drag drop\n"); fflush(stdout);
     OpaqueWidget::onDragDrop(e);
 }

void NoteDisplay::onButton(const event::Button &e)
{
   // printf("on button press=%d rel=%d\n", e.action == GLFW_PRESS, e.action==GLFW_RELEASE);   fflush(stdout);

    bool handled = false;

    const bool isPressed = e.action == GLFW_PRESS;
    const bool shift = e.mods & GLFW_MOD_SHIFT;
    const bool ctrl = e.mods & GLFW_MOD_CONTROL;

    if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
        handled = mouseManager-> onMouseButton(
            e.pos.x, 
            e.pos.y,
            isPressed, ctrl, shift);
    } else if (e.button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (isPressed && !shift && !ctrl) {
            sequencer->context->settings()->invokeUI(this);
            //seqSettings->invokeUI(this);
            handled = true;
        }
    }
    if (handled) {
        e.consume(this);
    } else {
        OpaqueWidget::onButton(e);
    }    
}

void NoteDisplay::onSelectKey(const event::SelectKey &e) 
{
    bool handled = handleKey(e.key, e.mods, e.action);
    if (handled) {
        e.consume(this);
    } else {
        OpaqueWidget::onSelectKey(e);
    }
}

bool NoteDisplay::isKeyWeNeedToStealFromRack(int key) {
    bool isCursor = false;
    switch (key) {
        case GLFW_KEY_LEFT:
        case GLFW_KEY_RIGHT:
        case GLFW_KEY_UP:
        case GLFW_KEY_DOWN:
        case GLFW_KEY_BACKSPACE:
        case GLFW_KEY_KP_DECIMAL:
        case GLFW_KEY_DELETE:
            isCursor = true;
    }
    return isCursor;
}

void NoteDisplay::onHoverKey(const event::HoverKey &e)
{
    bool handled = handleKey(e.key, e.mods, e.action);
    if (handled) {
        e.consume(this);
    } else if (isKeyWeNeedToStealFromRack(e.key)) {
        // Swallow all hover events around cursor keys.
        // This keeps Rack from stealing them.
        e.consume(this);
    } else {
        OpaqueWidget::onHoverKey(e);
    }
}

bool NoteDisplay::handleKey(int key, int mods, int action)
{
    bool handle = false;
    bool repeat = false;
    switch (action) {
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
        handle = MidiKeyboardHandler::doRepeat(key);
    }

    bool handled = false;
    if (handle) {
        handled = MidiKeyboardHandler::handle(sequencer, key, mods);
        if (handled) {
            rack::APP->event->setSelected(this);
        }
    }
    return handled;
}

void NoteDisplay::onSelect(const event::Select &e)
{
    updateFocus(true);
    e.consume(this);
}

void NoteDisplay::onDeselect(const event::Deselect &e)
{
    updateFocus(false);
    e.consume(this);
}

void NoteDisplay::onDragStart(const event::DragStart &e) 
{
    bool b = mouseManager->onDragStart();
    //printf("on drag start\n"); fflush(stdout);
    if (b) {
        e.consume(this);
    }
}
void NoteDisplay::onDragEnd(const event::DragEnd &e)
{
    //printf("on drag end\n"); fflush(stdout);
    bool b = mouseManager->onDragEnd();
     if (b) {
        e.consume(this);
    }
}
void NoteDisplay::onDragMove(const event::DragMove &e)
{
     bool b = mouseManager->onDragMove(e.mouseDelta.x, e.mouseDelta.y);
     if (b) {
        e.consume(this);
    }
}

#endif

//**************** All V0.6 keyboard handling here ***********

#ifndef __V1x

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
