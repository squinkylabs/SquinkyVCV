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
#include "MidiEditorContext.h"
#include "MidiSequencer.h"
#include <GLFW/glfw3.h>
#include "UIPrefs.h"
#include "MidiKeyboardHandler.h"
#include "NoteScreenScale.h"
#include "PitchUtils.h"
#include "../ctrl/SqHelper.h"
#include "TimeUtils.h"

#include <sstream>
#include "NoteDisplay.h"


NoteDisplay::NoteDisplay(const Vec& pos, const Vec& size, MidiSequencerPtr seq)
{
    this->box.pos = pos;
    box.size = size;
    sequencer = seq;

    if (sequencer) {
         initEditContext();
        scaler = std::make_shared<NoteScreenScale>(sequencer->context, size.x, size.y);
    }
    
    focusLabel = new Label();
    focusLabel->box.pos = Vec(40, 40);
    focusLabel->text = "";
    focusLabel->color = SqHelper::COLOR_WHITE;
    addChild(focusLabel);
    updateFocus(false);

        
    editAttributeLabel = new Label();
    editAttributeLabel->box.pos = Vec(10, 10);
    editAttributeLabel->text = "";
    editAttributeLabel->color = SqHelper::COLOR_WHITE;
    addChild(editAttributeLabel);

    barRangeLabel = new Label();
    barRangeLabel->box.pos = Vec(100, 10);
    barRangeLabel->text = "";
    barRangeLabel->color = SqHelper::COLOR_WHITE;
    addChild(barRangeLabel);
}

void NoteDisplay::setSequencer(MidiSequencerPtr seq) {
    sequencer = seq;
    sequencer->assertValid();
    initEditContext();
}

void NoteDisplay::initEditContext()
{
    // hard code view range (ofr now?)
    sequencer->context->setStartTime(0);
    sequencer->context->setEndTime(8);
    sequencer->context->setPitchLow(PitchUtils::pitchToCV(3, 0));
    sequencer->context->setPitchHi(PitchUtils::pitchToCV(5, 0));
}

 void NoteDisplay::step() 
{
    if (!sequencer) {
        return;
    }
    auto attr = sequencer->context->noteAttribute;
    if (curAttribute != attr) {
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
        }
    }

    int firstBar = 1 + TimeUtils::timeToBar(sequencer->context->startTime());
    if (firstBar != curFirstBar) {
        curFirstBar = firstBar;
        std::stringstream str;
        str << "First Bar: " << curFirstBar << " Last Bar: " << curFirstBar + 1;
        barRangeLabel->text = str.str();
    }
}

void NoteDisplay::drawNotes(NVGcontext *vg)
{
    MidiEditorContext::iterator_pair it = sequencer->context->getEvents();
    const int noteHeight = scaler->noteHeight();
    for ( ; it.first != it.second; ++it.first) {
        auto temp = *(it.first);
        MidiEventPtr evn = temp.second;
        MidiNoteEventPtr ev = safe_cast<MidiNoteEvent>(evn);

        const float x = scaler->midiTimeToX(*ev);
        const float y = scaler->midiPitchToY(*ev);
        const float width = scaler->midiTimeTodX(ev->duration);

        //  printf("draw note x=%f y=%f vs =%f\n", x, y, sequencer->context->viewport->startTime);
        fflush(stdout);

        const bool selected = sequencer->selection->isSelected(ev);
        filledRect(
            vg,
            selected ? UIPrefs::SELECTED_NOTE_COLOR : UIPrefs::NOTE_COLOR,
            x, y, width, noteHeight);
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
    filledRect(vg, UIPrefs::NOTE_EDIT_BACKGROUND, 0, 0, box.size.x, box.size.y);
    const int noteHeight = scaler->noteHeight();
    for (float cv = sequencer->context->pitchLow();
        cv <= sequencer->context->pitchHi();
        cv += PitchUtils::semitone) {
            const float y = scaler->midiCvToY(cv);
            const float width = box.size.x;
            bool accidental = PitchUtils::isAccidental(cv);
            if (accidental) {
                filledRect(
                    vg,
                    UIPrefs::NOTE_EDIT_ACCIDENTAL_BACKGROUND,
                    0, y, width, noteHeight);
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


#ifdef __V1
void NoteDisplay::onSelect(const event::Select &e) 
#else
void NoteDisplay::onFocus(EventFocus &e)
#endif
{
    updateFocus(true);
#ifdef __V1
        e.consume(this);
#else
        e.consumed = true;
#endif
}

#ifdef __V1
void NoteDisplay::onDeselect(const event::Deselect &e)
#else
void NoteDisplay::onDefocus(EventDefocus &e)
#endif
{
    updateFocus(false);
#ifdef __V1
        e.consume(this);
#else
        e.consumed = true;
#endif
}


#if 0
struct Key {
	/** GLFW_KEY_* */
	int key;
	/** GLFW_KEY_*. You should usually use `key` instead. */
	int scancode;
	/** GLFW_RELEASE, GLFW_PRESS, or GLFW_REPEAT */
	int action;
	/** GLFW_MOD_* */
	int mods;
};
#endif

#ifdef __V1
void NoteDisplay::onSelectKey(const event::SelectKey &e)
{
    // TODO: handle repeat
    if (e.action != GLFW_PRESS) {
        return;
    }
    bool handled = MidiKeyboardHandler::handle(sequencer.get(), e.key, e.mods);
    if (handled) {
        e.consume(this);
    } else {
         OpaqueWidget::onSelectKey(e);
    }
}

#else
void NoteDisplay::onKey(EventKey &e)
{
    const unsigned key = e.key;
    unsigned mods = 0;
    if (rack::windowIsShiftPressed()) {
        mods |= GLFW_MOD_SHIFT;
    }
    if ( windowIsModPressed()) {
        mods |= GLFW_MOD_CONTROL;
    }

    bool handled = MidiKeyboardHandler::handle(sequencer.get(), key, mods);
    if (!handled) {
        OpaqueWidget::onKey(e);
    } else {
        e.consumed = true;
    }

}

#endif
#endif
