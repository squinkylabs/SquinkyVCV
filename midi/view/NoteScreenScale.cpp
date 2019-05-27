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

    // now calculate the reverse function by just inverting the equation
    ax_rev = 1.0f / ax;
    bx_rev = -bx / ax;

    // third try
    ay_rev = -(ctx->pitchHi() - ctx->pitchLow()) / activeScreenHeight;

    // zero Y should be the highest pitch
    by_rev = ctx->pitchHi();
}


bool NoteScreenScale::isPointInBounds(float x, float y) const
{
    return (x >= hMargin) &&
        (x <= (screenWidth - hMargin)) &&
        (y > topMargin) &&
        (y < screenHeight);
        ;
}
float NoteScreenScale::midiTimeToX(const MidiEvent& ev) const
{
    return midiTimeToX(ev.startTime);
}

float NoteScreenScale::midiTimeToX(MidiEvent::time_t t) const
{
    return  bx + (t - context()->startTime()) * ax;
}

float NoteScreenScale::xToMidiTime(float x) const
{
    float t = bx_rev + ax_rev * x;
    t += context()->startTime();
    return t;
}

float NoteScreenScale::xToMidiDeltaTime(float x)
{
    return ax_rev * x;
}

float NoteScreenScale::midiTimeTodX(MidiEvent::time_t dt) const
{
    return  dt * ax;
}

float NoteScreenScale::midiPitchToY(const MidiNoteEvent& note) const
{
    return midiCvToY(note.pitchCV);
}


float NoteScreenScale::yToMidiCVPitch(float y) const
{
    float unquantizedPitch = (y - topMargin) * ay_rev + context()->pitchHi();
    std::pair<int, int> quantizedPitch = PitchUtils::cvToPitch(unquantizedPitch);
    return PitchUtils::pitchToCV(quantizedPitch.first, quantizedPitch.second);
}

float NoteScreenScale::yToMidiDeltaCVPitch(float dy) const
{
    return dy * ay_rev;
}


float NoteScreenScale::midiCvToY(float cv) const
{
    return by + (context()->pitchHi() - cv) * ay;
}

float NoteScreenScale::noteHeight() const
{
    return (1 / 12.f) * ay;
}

std::pair<float, float> NoteScreenScale::midiTimeToHBounds(const MidiNoteEvent& note) const
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

