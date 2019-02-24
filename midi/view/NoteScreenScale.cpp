#include "NoteScreenScale.h"
#include "MidiEditorContext.h"

NoteScreenScale::NoteScreenScale(MidiEditorContextPtr vp,
    float screenWidth,
    float screenHeight,
    float hMargin) : viewport(vp)
{
    assert(screenWidth > 0);
    assert(screenHeight > 0);
    viewport->assertValid();

    float activeScreenWidth = screenWidth - 2 * hMargin;

    ax = activeScreenWidth / (viewport->endTime() - viewport->startTime());
    bx = hMargin;

    // min and max the same is fine - it's just one note bar full screen
    ay = screenHeight / ((viewport->pitchHi() + 1 / 12.f) - viewport->pitchLow());
}

float NoteScreenScale::midiTimeToX(const MidiEvent& ev)
{
    return midiTimeToX(ev.startTime);
}

float NoteScreenScale::midiTimeToX(MidiEvent::time_t t)
{
    return  bx + (t - viewport->startTime()) * ax;
}

float NoteScreenScale::midiTimeTodX(MidiEvent::time_t dt)
{
    return  dt * ax;
}

float NoteScreenScale::midiPitchToY(const MidiNoteEvent& note)
{
    return (viewport->pitchHi() - note.pitchCV) * ay;
}

float NoteScreenScale::midiCvToY(float cv)
{
    return (viewport->pitchHi() - cv) * ay;
}

float NoteScreenScale::noteHeight()
{
    return (1 / 12.f) * ay;
}

std::pair<float, float> NoteScreenScale::midiTimeToHBounds(const MidiNoteEvent& note)
{
 //   float x = (note.startTime - viewport->startTime()) * ax;
    float x0 = midiTimeToX(note.startTime);
 //   float y = (note.startTime + note.duration - viewport->startTime()) * ax;
    float x1 = midiTimeToX(note.startTime + note.duration);
    return std::pair<float, float>(x0, x1);
}