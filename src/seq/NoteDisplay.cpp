#include "../Squinky.hpp"

#ifdef _SEQ
#include "WidgetComposite.h"
#include "Seq.h"


#include "MidiSequencer.h"
#include "UIPrefs.h"
#include "KbdManager.h"
#ifndef _USERKB
#include "MidiKeyboardHandler.h"
#endif
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
    ::rack::engine::Module* mod)
{
    this->box.pos = pos;
    box.size = size;
    sequencer = seq;
    mouseManager = std::make_shared<MouseManager>(sequencer);

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
#ifdef _USERKB
    kbdManager = std::make_shared<KbdManager>();
#endif
}


void NoteDisplay::songUpdated()
{
    initEditContext();
    // re-associate seq and mouse manager
    mouseManager = std::make_shared<MouseManager>(sequencer); 
}

void NoteDisplay::setSequencer(MidiSequencerPtr seq)
{
    assert(seq);
    sequencer = seq;
    sequencer->assertValid();
    songUpdated();
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

    const float endTime =  sequencer->context->getTrack()->getLength();
    const float endX = scaler->midiTimeToX(endTime);

    //assume two bars, quarter note grid
    const float totalDuration = TimeUtils::bar2time(2);
    float deltaDuration = sequencer->context->settings()->getQuarterNotesInGrid();

    // if grid lines are too close together, don't draw all.
    const float dx = scaler->midiTimeTodX(deltaDuration);
    if (dx < 22) {
        deltaDuration *= 2;
    }

    for (float relTime = 0; relTime <= totalDuration; relTime += deltaDuration) {
        const float time = relTime + sequencer->context->startTime();
        const float x = scaler->midiTimeToX(time);
        const float y = UIPrefs::topMarginNoteEdit;
        float width = 1;
        float height = this->box.size.y - y;

        const bool isBar = (relTime == 0) ||
            (relTime == TimeUtils::bar2time(1)) ||
            (relTime == TimeUtils::bar2time(2));

        NVGcolor color = isBar ? UIPrefs::GRID_BAR_COLOR : UIPrefs::GRID_COLOR;
        if (x == endX) {
            color = UIPrefs::GRID_END_COLOR;
        }
        SqGfx::filledRect(
            vg,
            color,
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

void NoteDisplay::draw(const Widget::DrawArgs &args)
{
    NVGcontext *vg = args.vg;
 
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
    OpaqueWidget::draw(args);
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

void NoteDisplay::onUIThread(std::shared_ptr<Seq<WidgetComposite>> seqComp, MidiSequencerPtr sequencer)
{
#ifdef _USERKB
    kbdManager->onUIThread(seqComp, sequencer);
#endif
}

/******************** All V1 keyboard handling here *******************
 *
 */

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
    const bool ctrl = e.mods & RACK_MOD_CTRL;

    if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
        handled = mouseManager->onMouseButton(
            e.pos.x,
            e.pos.y,
            isPressed, ctrl, shift);
    } else if (e.button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (isPressed && !shift && !ctrl) {
            // first, click here to get note select status ready for new commands
            mouseManager->onMouseButton(
                e.pos.x,
                e.pos.y,
                isPressed, ctrl, shift);

            // now invoke the settings menu
            sequencer->context->settings()->invokeUI(this);
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

bool NoteDisplay::isKeyWeNeedToStealFromRack(int key)
{
#ifdef _USERKB
    if (!kbdManager->shouldGrabKeys())
    {
        return false;
    }
#endif
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
  //  fprintf(stderr, "\n** NoteDisplay::handleKey, action = %d (press = %d, repeat = %d\n", action,
  //      GLFW_PRESS, GLFW_REPEAT);
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
        // TODO: how will we handle repeat in the _USERKB work
       
#ifdef _USERKB
        handle = true;
#else
        handle = MidiKeyboardHandler::doRepeat(key);
#endif
    }

    bool handled = false;
    if (handle) {
#ifdef _USERKB
        handled = kbdManager->handle(sequencer, key, mods);
#else
        handled = MidiKeyboardHandler::handle(sequencer, key, mods);
#endif
        if (handled) {
            ::rack::appGet()->event->setSelected(this);
        }
    }
   // fprintf(stderr, "NoteDisplay::handleKey ret = %d\n", handled);
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
    if (b) {
        e.consume(this);
    }
}
void NoteDisplay::onDragEnd(const event::DragEnd &e)
{
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
