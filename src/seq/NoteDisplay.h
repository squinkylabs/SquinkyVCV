
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
        printf("in ctor, seq = %p\n", sequencer.get()); fflush(stdout);
        printf("in ctor, seq-<context = %p\n", sequencer->context.get()); fflush(stdout);
        
        printf("in ctor, seqvp = %p\n", sequencer->context->viewport.get()); fflush(stdout);
        
        auto s = sequencer->context->viewport->_song.lock();
        printf("in ctor, seq vp song s = %p\n", s.get()); fflush(stdout);
        
        //viewport._song = song;

        // hard code view range to our demo song
        sequencer->context->viewport->startTime = 0;
        sequencer->context->viewport->endTime =
        sequencer->context->viewport->startTime + 8;
        sequencer->context->viewport->pitchLow = PitchUtils::pitchToCV(3, 0);
        sequencer->context->viewport->pitchHi = PitchUtils::pitchToCV(5, 0);

        //initScaleFuncs();
        scaler = std::make_shared<NoteScreenScale>(sequencer->context->viewport, size.x, size.y);
    }

    std::shared_ptr<NoteScreenScale> scaler;

    float ax =0;
    float ay=0;

    MidiSequencerPtr sequencer;

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

    void draw(NVGcontext *vg) override
    {   
#if 0
        filledRect(vg, red, 50, 50, 30, 30); 
        filledRect(vg, blue, 0, 0, 30, 30); 
        strokedRect(vg,  nvgRGBA(0x00, 0xff, 0x00, 0xff), 100, 100, 40, 10);
        strokedRect(vg, green, 0, 0, this->box.size.x, this->box.size.y);
        filledRect(vg, blue, this->box.size.x - 10, 100, 100, 10);
#endif
        drawBackground(vg);
        drawNotes(vg);
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
        std::cout << "onMouseDown " << e.button << std::flush << std::endl;
    }
    void onMouseMove(EventMouseMove &e) override
    {
       //  std::cout << "onMouseMove " << std::flush << std::endl;
        OpaqueWidget::onMouseMove(e);
    }
    void onFocus(EventFocus &e) override
    {
        std::cout << "onFocus " << std::flush << std::endl;
      // OpaqueWidget::onFocus(e);
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
    //virtual void onFocus(EventFocus &e) {}
    void onDefocus(EventDefocus &e) override

    {
       // std::cout << "defoucs " << std::flush << std::endl;
        e.consumed = true;
    }
};
