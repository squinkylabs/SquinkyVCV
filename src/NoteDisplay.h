
#pragma once
#include "nanovg.h"
#include "MidiViewport.h"


class NoteScreenScale
{
public:
    NoteScreenScale(MidiViewport& vp, const Vec& screenSize) : viewport(vp)
    {
        ax =screenSize.x / (viewport.endTime - viewport.startTime);
        ay = screenSize.y / (viewport.pitchHi - viewport.pitchLow);

        printf("in init ax=%f ay=%f screenx=%f screeny=%f\n", ax, ay, screenSize.x, screenSize.y);
        fflush(stdout);
    }
    float midiTimeToX(const MidiEvent& ev)
    {
        return (ev.startTime - viewport.startTime) * ax;  
    }
     float midiTimeTodX(MidiEvent::time_t dt)
    {
        return  dt * ax;
    }
    float midiPitchToY(const MidiNoteEvent& note)
    {
       // return (note.pitchCV - viewport.pitchLow) * ay;
        return ( -1.f/12.f + viewport.pitchHi - note.pitchCV) * ay;
    }
private:
    float ax = 0;
    float ay = 0;
    MidiViewport& viewport;
};

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

        // hard code view range to our demo song
        viewport.startTime = 0;
        viewport.endTime = viewport.startTime + 8;
        viewport.pitchLow = MidiNoteEvent::pitchToCV(3, 0);
        viewport.pitchHi = MidiNoteEvent::pitchToCV(5, 0);

        //initScaleFuncs();
        scaler = std::make_shared<NoteScreenScale>(viewport, size);
    }

    std::shared_ptr<NoteScreenScale> scaler;

    float ax =0;
    float ay=0;
    const NVGcolor red =  nvgRGBA(0xff, 0x00, 0x00, 0xff); 
    const NVGcolor green =  nvgRGBA(0x00, 0xff, 0x00, 0xff); 
    const NVGcolor blue =  nvgRGBA(0x00, 0x00, 0xff, 0xff); 
    const NVGcolor bkgnd =  nvgRGBA(0xdd, 0xdd, 0xdd, 0xff); 

    MidiViewport viewport;
    MidiSongPtr song;

    void drawNotes(NVGcontext *vg)
    {
        MidiViewport::iterator_pair it = viewport.getEvents();
        for ( ; it.first != it.second; ++it.first) {
            auto temp = *(it.first);
            MidiEventPtr evn = temp.second;
            MidiNoteEventPtr ev = safe_cast<MidiNoteEvent>(evn);

            const float x = scaler->midiTimeToX(*ev);
            const float y = scaler->midiPitchToY(*ev);
            const float width = scaler->midiTimeTodX(ev->duration);

            filledRect(vg, red, x, y, width, 10);
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
        filledRect(vg, bkgnd, 0, 0, box.size.x, box.size.y);
    
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
