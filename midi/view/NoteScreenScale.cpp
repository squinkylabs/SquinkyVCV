#include "NoteScreenScale.h"
#include "MidiViewport.h"

NoteScreenScale::NoteScreenScale(MidiViewportPtr vp, float screenWidth, float screenHeight) : viewport(vp)
{
    assert(screenWidth > 0);
    assert(screenHeight > 0);
    assert(viewport->pitchHi >= viewport->pitchLow);
    ax = screenWidth / (viewport->endTime - viewport->startTime);

    // min and max the same is fine - it's just one note bar full screen
    float dbg = ((viewport->pitchHi + 1 / 12.f) - viewport->pitchLow);
    ay = screenHeight / ((viewport->pitchHi + 1 / 12.f) - viewport->pitchLow);

    //printf("in init ax=%f ay=%f screenx=%f screeny=%f\n", ax, ay, screenWidth, screenHeight);
    //fflush(stdout);
}

float NoteScreenScale::midiTimeToX(const MidiEvent& ev)
{
    return (ev.startTime - viewport->startTime) * ax;
}

float NoteScreenScale::midiTimeTodX(MidiEvent::time_t dt)
{
    return  dt * ax;
}

float NoteScreenScale::midiPitchToY(const MidiNoteEvent& note)
{
    return (viewport->pitchHi - note.pitchCV) * ay;
}

float NoteScreenScale::midiCvToY(float cv)
{
    return (viewport->pitchHi - cv) * ay;
}

float NoteScreenScale::noteHeight()
{
    return (1 / 12.f) * ay;
}

std::pair<float, float> NoteScreenScale::midiTimeToHBounds(const MidiNoteEvent& note)
{
    float x = (note.startTime - viewport->startTime) * ax;
    float y = (note.startTime + note.duration - viewport->startTime) * ax;
    return std::pair<float, float>(x, y);
}