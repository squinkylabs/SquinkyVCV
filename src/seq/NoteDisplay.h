
#pragma once
#include "util/math.hpp"
#include "nanovg.h"
#include "window.hpp"
#include "MidiViewport.h"
#include "MidiSequencer.h"
#include <GLFW/glfw3.h>
#include "UIPrefs.h"
#include "MidiKeyboardHandler.h"
#include "NoteScreenScale.h"
#include "PitchUtils.h"
#include "../ctrl/SqHelper.h"


/**
 * This class needs some refactoring and renaming.
 * It is really the entire sequencer UI, including the notes.
 * 
 * Pretty soon we should sepparate out the NoteEditor.
 */
struct NoteDisplay : OpaqueWidget
{
    NoteDisplay(const Vec& pos, const Vec& size, MidiSongPtr song)
    {
        this->box.pos = pos;
		box.size = size;
        sequencer = std::make_shared<MidiSequencer>(song);
        
        assert(sequencer->context->vieport._song == song);
   
        // hard code view range to our demo song
        sequencer->context->viewport->startTime = 0;
        sequencer->context->viewport->endTime =
        sequencer->context->viewport->startTime + 8;
        sequencer->context->viewport->pitchLow = PitchUtils::pitchToCV(3, 0);
        sequencer->context->viewport->pitchHi = PitchUtils::pitchToCV(5, 0);

        //initScaleFuncs();
        scaler = std::make_shared<NoteScreenScale>(sequencer->context->viewport, size.x, size.y);
        
        infoLabel = new Label();
        infoLabel->box.pos = Vec(10, 10);
        infoLabel->text = "";
        infoLabel->color = SqHelper::COLOR_WHITE;
        addChild(infoLabel);
        updateFocus(false);
    }

    Label* infoLabel;
    std::shared_ptr<NoteScreenScale> scaler;
    MidiSequencerPtr sequencer;
    bool cursorState = false;
    int cursorFrameCount = 0;
    bool haveFocus = true;

    void updateFocus(bool focus) {
        if (focus != haveFocus) {
            haveFocus = focus;
            infoLabel->text = focus ? "" : "Click in editor to get focus";
        }
    }

    void drawNotes(NVGcontext *vg)
    {
        MidiViewport::iterator_pair it = sequencer->context->viewport->getEvents();
        const int noteHeight = scaler->noteHeight();
        for ( ; it.first != it.second; ++it.first) {
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


    void drawCursor(NVGcontext *vg) {
        cursorFrameCount--;
        if (cursorFrameCount < 0) {
            cursorFrameCount = 10;
            cursorState = !cursorState;
        }

        if (true) {
            auto color = cursorState ? 
                nvgRGB(0xff, 0xff, 0xff) :
                nvgRGB(0, 0, 0);
            const float x = scaler->midiTimeTodX(sequencer->context->cursorTime);
                       
            const float y = scaler->midiCvToY(sequencer->context->cursorPitch) + 
                 scaler->noteHeight() / 2.f;  
            filledRect(vg, color, x, y, 10, 3);
        }

    }
    void draw(NVGcontext *vg) override
    {   
        drawBackground(vg);
        drawNotes(vg);
        drawCursor(vg);
        OpaqueWidget::draw(vg);
    }

    void drawBackground(NVGcontext *vg) {
        filledRect(vg, UIPrefs::NOTE_EDIT_BACKGROUND, 0, 0, box.size.x, box.size.y);
        const int noteHeight = scaler->noteHeight();
        for (float cv = sequencer->context->viewport->pitchLow;
            cv <= sequencer->context->viewport->pitchHi;
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

   void strokedRect(NVGcontext *vg, NVGcolor color, float x, float y, float w, float h)
    {
        nvgStrokeColor(vg, color);
        nvgBeginPath(vg);
        nvgRect(vg, x, y, w, h);
        nvgStroke(vg);
    }

    void filledRect(NVGcontext *vg, NVGcolor color, float x, float y, float w, float h)
    {
        nvgFillColor(vg, color);
        nvgBeginPath(vg);
        nvgRect(vg, x, y, w, h);
        nvgFill(vg);
    }
  
  //************************** These overrides are just to test even handling
    void onMouseDown(EventMouseDown &e) override
    {
        OpaqueWidget::onMouseDown(e);
      //  std::cout << "onMouseDown " << e.button << std::flush << std::endl;
    }
    void onMouseMove(EventMouseMove &e) override
    {
       //  std::cout << "onMouseMove " << std::flush << std::endl;
        OpaqueWidget::onMouseMove(e);
    }
    void onFocus(EventFocus &e) override
    {
        updateFocus(true);
        e.consumed = true;
    }
    void onDefocus(EventDefocus &e) override
    {
        updateFocus(false);
        e.consumed = true;
    }
    void onText(EventText &e) override
    {
        std::cout << "onText " << std::flush << std::endl;
        OpaqueWidget::onText(e);
    }

    void onKey(EventKey &e) override
    {
        const unsigned key = e.key;
        unsigned mods = 0;
        if (rack::windowIsShiftPressed()) {
            mods |= GLFW_MOD_SHIFT;
        }
        if ( windowIsModPressed()) {
            mods |= GLFW_MOD_CONTROL;
        }

        bool handled =MidiKeyboardHandler::handle(sequencer.get(), key, mods);
        if (!handled) {
            OpaqueWidget::onKey(e);
        }
    }
    void onMouseEnter(EventMouseEnter &e) override
    {
        //std::cout << "nmouseenger " << std::flush << std::endl;
    }
   /** Called when another widget begins responding to `onMouseMove` events */
//	virtual void onMouseLeave(EventMouseLeave &e) {}

};
