#include "NoteScreenScale.h"
#include "MidiEditorContext.h"

NoteScreenScale::NoteScreenScale(
    float screenWidth,
    float screenHeight,
    float hMargin,
    float topMargin) :
        screenWidth(screenWidth),
        screenHeight(screenHeight),
        hMargin(hMargin),
        topMargin(topMargin)
{
    assert(screenWidth > 0);
    assert(screenHeight > 0);
}

void NoteScreenScale::setContext(std::shared_ptr<MidiEditorContext> context)
{
    assert( context->pitchLow() <= context->pitchHi());
    _context = context;
    this->context()->assertValid();
    reCalculate();
}

void NoteScreenScale::assertValid() const
{
    assert(this->context());
}


void NoteScreenScale::reCalculate()
{
    const float activeScreenWidth = screenWidth - 2 * hMargin;
    auto ctx = context();
    ax = activeScreenWidth / (ctx->endTime() - ctx->startTime());
    bx = hMargin;

    // min and max the same is fine - it's just one note bar full screen
    float activeScreenHeight = screenHeight - topMargin;
    ay = activeScreenHeight / ((ctx->pitchHi() + 1 / 12.f) - ctx->pitchLow());
    by = topMargin;

    assert( ctx->pitchLow() <= ctx->pitchHi());
}

float NoteScreenScale::midiTimeToX(const MidiEvent& ev)
{
    return midiTimeToX(ev.startTime);
}

float NoteScreenScale::midiTimeToX(MidiEvent::time_t t)
{
    return  bx + (t - context()->startTime()) * ax;
}

float NoteScreenScale::midiTimeTodX(MidiEvent::time_t dt)
{
    return  dt * ax;
}

float NoteScreenScale::midiPitchToY(const MidiNoteEvent& note)
{
    return midiCvToY(note.pitchCV);
}

float NoteScreenScale::midiCvToY(float cv)
{
    return by + (context()->pitchHi() - cv) * ay;
}

float NoteScreenScale::noteHeight()
{
    return (1 / 12.f) * ay;
}

std::pair<float, float> NoteScreenScale::midiTimeToHBounds(const MidiNoteEvent& note)
{
    float x0 = midiTimeToX(note.startTime);
    float x1 = midiTimeToX(note.startTime + note.duration);
    return std::pair<float, float>(x0, x1);
}

std::shared_ptr<MidiEditorContext> NoteScreenScale::context() const
{
    std::shared_ptr<MidiEditorContext> ret = _context.lock();
    assert(ret);
    return ret;
}