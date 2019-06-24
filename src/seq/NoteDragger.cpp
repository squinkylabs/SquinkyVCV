

#include "../Squinky.hpp"
#include "SqGfx.h"
#include "WidgetComposite.h"

#ifdef __V1x
    #include "widget/Widget.hpp"
    #include "app.hpp"
#else
    #include "widgets.hpp"
    #include "util/math.hpp"
    #include "window.hpp"
#endif

#include "NoteDragger.h"
#include "NoteDisplay.h"
#include "UIPrefs.h"

NoteDragger::NoteDragger(MidiSequencerPtr seq, float initX, float initY) :
    sequencer(seq),
    startX(initX),
    startY(initY)
{ 
    curMousePositionX = initX;
    curMousePositionY = initY;
}

NoteDragger::~NoteDragger()
{
}

void NoteDragger::onDrag(float deltaX, float deltaY)
{
    curMousePositionX += deltaX;    
    curMousePositionY += deltaY;
}

void NoteDragger::drawNotes(NVGcontext *vg, float verticalShift, float horizontalShift, float horizontalStretch)
{
    auto scaler = sequencer->context->getScaler();
    assert(scaler);

    MidiEditorContext::iterator_pair it = sequencer->context->getEvents();
    
    const int noteHeight = scaler->noteHeight();
    for (; it.first != it.second; ++it.first) {
        auto temp = *(it.first);
        MidiEventPtr evn = temp.second;
        MidiNoteEventPtr ev = safe_cast<MidiNoteEvent>(evn);

        const float x = scaler->midiTimeToX(*ev) + horizontalShift;
        const float y = scaler->midiPitchToY(*ev) + verticalShift;
        const float width = scaler->midiTimeTodX(ev->duration) + horizontalStretch;

        const bool selected = sequencer->selection->isSelected(ev);
        if (selected) {
            SqGfx::filledRect(
                vg,
                UIPrefs::SELECTED_NOTE_COLOR,
                x, y, width, noteHeight);
        }
    }
}

/******************************************************************
 *
 * NotePitchDragger 
 */

// Remember current viewport pitch range. Shave some off top and
// bottom to allow reasonable dragging.
NotePitchDragger::NotePitchDragger(MidiSequencerPtr seq, float x, float y) :
    NoteDragger(seq, x, y),
    viewportUpperPitch0(sequencer->context->pitchHigh()),
    highPitchForDragStart(sequencer->context->pitchHigh() - 2 * PitchUtils::semitone),
    viewportLowerPitch0(sequencer->context->pitchLow()),
    lowPitchForDragStart(sequencer->context->pitchLow() + 2 * PitchUtils::semitone),
    pitch0(sequencer->context->getScaler()->yToMidiCVPitch(y))
{
}

float NotePitchDragger::calcTranspose() const
{
    auto scaler = sequencer->context->getScaler();
    const float verticalShift =  curMousePositionY - startY;
    const float transposeCV = scaler->yToMidiDeltaCVPitch(verticalShift);
    return transposeCV;
}

// TODO: take into account note height!
float NotePitchDragger::calcShift(float transpose) const
{
    auto scaler = sequencer->context->getScaler();
    assert(scaler);

    float ret = 0;
    #if 0
    printf("in calcShift, t=%.2f, draghi=%.2f draglo=%.2f\n", transpose, highPitchForDragStart, lowPitchForDragStart);
    printf("  viewport hi = %.2f low = %.2f initialPitch = %.2f\n", 
        viewportUpperPitch0,
        viewportLowerPitch0,
        pitch0);
    #endif

    // distance between initial mouse click and top of viewport
    const float deltaP02Hp = highPitchForDragStart - pitch0; 
    const float deltaP02Lp = lowPitchForDragStart - pitch0; 

    if (transpose > deltaP02Hp) {
        ret = transpose - deltaP02Hp;
    } else if (transpose < deltaP02Lp) {
        ret =  transpose - deltaP02Lp;
    }

    return ret;
}

void NotePitchDragger::onDrag(float deltaX, float deltaY)
{
    NoteDragger::onDrag(deltaX, deltaY);
    const float transpose = calcTranspose();
    const float shift = calcShift(transpose);

   // printf("onDrag, trans = %.2f, shift = %.2f\n", transpose, shift); fflush(stdout);

    // TODO: only if shift moves away from center,
    // or only if pitch not in viewport.
    auto scaler = sequencer->context->getScaler();
    if (shift) {
        sequencer->context->setPitchRange(viewportLowerPitch0 + shift, viewportUpperPitch0 + shift);
    }
}

void NotePitchDragger::commit()
{
    // TODO: use calcTranspose
    auto scaler = sequencer->context->getScaler();
    const float verticalShift =  curMousePositionY - startY;
    const float transposeCV = scaler->yToMidiDeltaCVPitch(verticalShift);
    const int semiShift = PitchUtils::deltaCVToSemitone(transposeCV);
    if (semiShift != 0) {
        // only do the edit if significant change
        sequencer->editor->changePitch(semiShift);
    }
}

void NotePitchDragger::draw(NVGcontext *vg)
{
    float verticalShift =  curMousePositionY - startY;
    drawNotes(vg, verticalShift, 0, 0);
    SqGfx::drawText(vg, curMousePositionX + 20, curMousePositionY + 20, "transpose");
}

/******************************************************************
 *
 * HorizontalDragger 
 */

/*

   viewportUpperPitch0(sequencer->context->pitchHi()),
    highPitchForDragStart(sequencer->context->pitchHi() - 2 * PitchUtils::semitone),
    viewportLowerPitch0(sequencer->context->pitchLow()),
    lowPitchForDragStart(sequencer->context->pitchLow() + 2 * PitchUtils::semitone),
    pitch0(sequencer->context->getScaler()->yToMidiCVPitch(y))
 */

NoteHorizontalDragger::NoteHorizontalDragger(MidiSequencerPtr seq, float x, float y) :
    NoteDragger(seq, x, y),
    viewportStartTime0(sequencer->context->startTime()),
    viewportEndTime0(sequencer->context->endTime()),
    time0(sequencer->context->getScaler()->xToMidiTime(x))
{

}

/******************************************************************
 *
 * NoteStartDragger 
 */
NoteStartDragger::NoteStartDragger(MidiSequencerPtr seq, float x, float y) :
    NoteHorizontalDragger(seq, x, y)
{ 
}

void NoteStartDragger::draw(NVGcontext *vg)
{
    const float horizontalShift =  curMousePositionX - startX;
    drawNotes(vg, 0, horizontalShift, 0);
    SqGfx::drawText(vg, curMousePositionX + 20, curMousePositionY + 20, "shift");
}

void NoteStartDragger::commit()
{
    printf("NoteStartDragger::commit\n"); fflush(stdout);
    auto scaler = sequencer->context->getScaler();
    const float horizontalShift =  curMousePositionX - startX;
    const float timeShiftAmount = scaler->xToMidiDeltaTime(horizontalShift);

    // convert qusrter notes to 64th notes.
    const int timeShiftTicks = std::round(timeShiftAmount * 16);

    if (timeShiftTicks != 0) {
        sequencer->editor->changeStartTime(true, timeShiftTicks);
    }
} 


/******************************************************************
 *
 * NoteDurationDragger 
 */

NoteDurationDragger::NoteDurationDragger(MidiSequencerPtr seq, float x, float y) :
    NoteHorizontalDragger(seq, x, y)
{
}

void NoteDurationDragger::draw(NVGcontext *vg)
{
    const float horizontalShift =  curMousePositionX - startX;
    drawNotes(vg, 0, 0, horizontalShift);
    SqGfx::drawText(vg, curMousePositionX + 20, curMousePositionY + 20, "stretch");
}

void NoteDurationDragger::commit()
{
    auto scaler = sequencer->context->getScaler();
    const float horizontalShift =  curMousePositionX - startX;
    const float timeShiftAmount = scaler->xToMidiDeltaTime(horizontalShift);
    const int timeShiftTicks = std::round(timeShiftAmount * 16);
    if (timeShiftTicks != 0) {
        sequencer->editor->changeDuration(true, timeShiftTicks);
    }
}

