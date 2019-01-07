
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
    NoteDisplay(const Vec& pos, const Vec& size)
    {
        this->box.pos = pos;
		box.size = size;
        song = MidiSong::makeTest1();
        viewport._song = song;
        viewport.startTime = startTime;
        viewport.endTime = startTime + totalDuration;

        viewport.pitchLow = MidiNoteEvent::pitchToCV(3, 0);
        viewport.pitchHi = MidiNoteEvent::pitchToCV(5, 0);

        initScaleFuncs();
    }


    // put in something to define the range we
    // want to display
    const float startTime = 0;
    const float totalDuration = 8;
   // float pitchLow=0;
  //  float pitchHigh=0;


    float ax =0;
    float ay=0;
    const NVGcolor red =  nvgRGBA(0xff, 0x00, 0x00, 0xff); 
    const NVGcolor green =  nvgRGBA(0x00, 0xff, 0x00, 0xff); 
    const NVGcolor blue =  nvgRGBA(0x00, 0x00, 0xff, 0xff); 

    void initScaleFuncs()
    {
        printf("in initscal tot=%f, p=%f, %f\n",
            totalDuration, viewport.pitchHi, viewport.pitchLow);
        printf("box size = %f, %f\n", this->box.size.x, this->box.size.y);
        ax = this->box.size.x / totalDuration;
        ay = this->box.size.y / (viewport.pitchHi - viewport.pitchLow);
    }
    float midiTimeToX(const MidiEvent& ev)
    {
        return (ev.startTime - startTime) * ax;  
    }

    float midiTimeTodX(MidiEvent::time_t dt)
    {
        return  dt * ax;
    }
    float midiPitchToY(const MidiNoteEvent& note)
    {
        return (note.pitchCV - viewport.pitchLow) * ay;
    }

    MidiViewport viewport;
    MidiSongPtr song;


    void drawNotes(NVGcontext *vg)
    {
        MidiViewport::iterator_pair it = viewport.getEvents();
        for ( ; it.first != it.second; ++it.first) {
            auto temp = *(it.first);
            MidiEventPtr evn = temp.second;
            MidiNoteEventPtr ev = safe_cast<MidiNoteEvent>(evn);

            const float x = midiTimeToX(*ev);
            const float y = midiPitchToY(*ev);
            const float width = midiTimeTodX(ev->duration);


            filledRect(vg, red, x, y, width, 10);
        }
    }

    void draw(NVGcontext *vg) override
    {
        // draw some squares for fun
       // nvgScale(vg, 2, 2);

     

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
