#include "NoteScreenScale.h"
#include "MidiViewport.h"

NoteScreenScale::NoteScreenScale(MidiViewport& vp, float screenWidth, float screenHeight) : viewport(vp)
{
    assert(screenWidth > 0);
    assert(screenHeight > 0);
    ax =screenWidth / (viewport.endTime - viewport.startTime);
    ay = screenHeight / (viewport.pitchHi - viewport.pitchLow);

    //printf("in init ax=%f ay=%f screenx=%f screeny=%f\n", ax, ay, screenWidth, screenHeight);
    //fflush(stdout);
}

float NoteScreenScale::midiTimeToX(const MidiEvent& ev)
{
    return (ev.startTime - viewport.startTime) * ax;  
}

float NoteScreenScale::midiTimeTodX(MidiEvent::time_t dt)
{
    return  dt * ax;
}

float NoteScreenScale::midiPitchToY(const MidiNoteEvent& note)
{
    // return (note.pitchCV - viewport.pitchLow) * ay;
    return ( -1.f/12.f + viewport.pitchHi - note.pitchCV) * ay;
}