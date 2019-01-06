
#pragma once
#include "nanovg.h"
#include "MidiViewport.h"

/**
 * Experiments:
 * 
 * refactor to remove redundant code
 * draw stroked rectangle
 * figure out how to normalize the coordinates.
 */
struct NoteDisplay : OpaqueWidget
{
    NoteDisplay()
    {
        song = MidiSong::makeTest1();
        viewport._song = song;
        viewport.startTime = startTime;
        viewport.endTime = startTime + totalDuration;

        MidiNoteEvent note;
        note.setPitch(4, 0);

        viewport.pitchLow = pitchLow = note.pitch;
        note.setPitch(6, 0);
        viewport.pitchHi = pitchHigh = note.pitch;
    }


    // put in something to define the range we
    // want to display
    const float startTime = 0;
    const float totalDuration = 8;
    float pitchLow=0;
    float pitchHigh=0;

    float midiTimeToX(const MidiEvent& ev);
    float midiPitchToY(const MidiNoteEvent& note);

    MidiViewport viewport;
    MidiSongPtr song;

    bool did = false;
    void drawNotes(NVGcontext *vg)
    {
        if (!did) {
            printf("draw notes called\n");
             fflush(stdout);
        }
        MidiViewport::iterator_pair it = viewport.getEvents();
        for ( ; it.first != it.second; ++it.first) {
            if (!did) {
               // const MidiEvent& evn = *(it.first);
               auto temp = *(it.first);
               MidiEventPtr evn = temp.second;
               MidiNoteEventPtr ev = safe_cast<MidiNoteEvent>(evn);
                
                printf("iter note, pitch = %d\n",ev->getPitch().second);
                fflush(stdout);

            }
        }
        if (!did) {
 printf("draw notes done\n");
             fflush(stdout);
        }
        did = true;
    }

    void draw(NVGcontext *vg) override
    {
        // draw some squares for fun
       // nvgScale(vg, 2, 2);

        const auto red =  nvgRGBA(0xff, 0x00, 0x00, 0xff); 
        const auto green =  nvgRGBA(0x00, 0xff, 0x00, 0xff); 
        const auto blue =  nvgRGBA(0x00, 0x00, 0xff, 0xff); 

        filledRect(vg, red, 50, 50, 30, 30); 
        filledRect(vg, blue, 0, 0, 30, 30); 

        strokedRect(vg,  nvgRGBA(0x00, 0xff, 0x00, 0xff), 100, 100, 40, 10);

        strokedRect(vg, green, 0, 0, this->box.size.x, this->box.size.y);

        // this oun goes out of the bounds
        filledRect(vg, blue, this->box.size.x - 10, 100, 100, 10);
    
        drawNotes(vg);
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
        std::cout << "onKey " << e.key << std::flush << std::endl;
        if (!e.consumed) {
            OpaqueWidget::onKey(e);
        }
    }
    void onMouseEnter(EventMouseEnter &e) override
    {
        std::cout << "nmouseenger " << std::flush << std::endl;
    }
   /** Called when another widget begins responding to `onMouseMove` events */
//	virtual void onMouseLeave(EventMouseLeave &e) {}
    //virtual void onFocus(EventFocus &e) {}
    void onDefocus(EventDefocus &e) override

    {
        std::cout << "defoucs " << std::flush << std::endl;
        e.consumed = true;
    }
};
