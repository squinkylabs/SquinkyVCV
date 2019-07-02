

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

#include "ISeqSettings.h"
#include "MidiEvent.h"
#include "MidiSequencer.h"
#include "NoteDragger.h"
#include "NoteDisplay.h"
#include "TimeUtils.h"
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

// all of the "shift" and drag params are in pixels
void NoteDragger::drawNotes(NVGcontext *vg, float verticalShift, float horizontalShift, float horizontalStretch)
{
    auto scaler = sequencer->context->getScaler();
    assert(scaler);
    const int noteHeight = scaler->noteHeight();

    // For drag operations, let's use the selection to pick notes to draw.
    for (auto it : *sequencer->selection) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(it);
        if (note) {
            float quantizedHShift = quantizeForDisplay(*note, horizontalShift);
            const float x = scaler->midiTimeToX(*note) + quantizedHShift;
            const float y = scaler->midiPitchToY(*note) + verticalShift;
            const float width = scaler->midiTimeTodX(note->duration) + horizontalStretch;

            //printf("drawing at x=%.2f y = %.2f width = %.2f h=%d\n", x, y, width, noteHeight);
           // fflush(stdout);

            SqGfx::filledRect(
                vg,
                UIPrefs::SELECTED_NOTE_COLOR,
                x, y, width, noteHeight);
        }
    }
}

float NoteDragger::quantizeForDisplay(const MidiNoteEvent& note, float timeShiftPixels)
{
   return timeShiftPixels;       // default imp does nothing
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
    const float verticalShift = curMousePositionY - startY;
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
        ret = transpose - deltaP02Lp;
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
    sequencer->context->setCursorPitch(transpose + pitch0);
    //printf("just set cursor pitch to %.2f\n", transpose + pitch0); fflush(stdout);
}

void NotePitchDragger::commit()
{
    //printf("enter commit-1, cursor pitch = %f\n", sequencer->context->cursorPitch());
    // TODO: use calcTranspose
    auto scaler = sequencer->context->getScaler();
    const float verticalShift = curMousePositionY - startY;
    const float transposeCV = scaler->yToMidiDeltaCVPitch(verticalShift);
    const int semiShift = PitchUtils::deltaCVToSemitone(transposeCV);

    // Only do the edit if significant change.
    if (semiShift != 0) {
        // Restore cursor to original pitch.
        sequencer->context->setCursorPitch(pitch0);
        // Now transpose notes and cursor.
        sequencer->editor->changePitch(semiShift);
    }
    // printf("leave commit-2, cursor pitch = %f\n", sequencer->context->cursorPitch());
    // fflush(stdout);
}

void NotePitchDragger::draw(NVGcontext *vg)
{
    float verticalShift = curMousePositionY - startY;
    drawNotes(vg, verticalShift, 0, 0);
    SqGfx::drawText(vg, curMousePositionX + 20, curMousePositionY + 20, "transpose");
}

/******************************************************************
 *
 * HorizontalDragger
 */

NoteHorizontalDragger::NoteHorizontalDragger(MidiSequencerPtr seq, float x, float y, float initialNoteValue) :
    NoteDragger(seq, x, y),
    viewportStartTime0(sequencer->context->startTime()),
    viewportEndTime0(sequencer->context->endTime()),
    time0(sequencer->context->getScaler()->xToMidiTime(x)),
    initialNoteValue(initialNoteValue)
{
}

float NoteHorizontalDragger::calcTimeShift() const
{
    auto scaler = sequencer->context->getScaler();
    const float horizontalShift = curMousePositionX - startX;
    const float timeShiftAmount = scaler->xToMidiDeltaTime(horizontalShift);
    return timeShiftAmount;
}

void NoteHorizontalDragger::onDrag(float deltaX, float deltaY)
{
    NoteDragger::onDrag(deltaX, deltaY);
    const float timeShift = calcTimeShift();

    const float t = timeShift + time0;

    auto x = TimeUtils::time2barsAndRemainder(2, t);
    const float newStartTime = std::get<0>(x) * TimeUtils::bar2time(2);
    const float newEndTime = newStartTime + TimeUtils::bar2time(2);
    sequencer->context->setTimeRange(newStartTime, newEndTime);
    sequencer->context->setCursorTime(t);
}

/******************************************************************
 *
 * NoteStartDragger
 */
NoteStartDragger::NoteStartDragger(MidiSequencerPtr seq, float x, float y, float noteStartTime) :
    NoteHorizontalDragger(seq, x, y, noteStartTime)
{
}

void NoteStartDragger::draw(NVGcontext *vg)
{
    const float horizontalShift = curMousePositionX - startX;
    drawNotes(vg, 0, horizontalShift, 0);
    SqGfx::drawText(vg, curMousePositionX + 20, curMousePositionY + 20, "shift");
}

float NoteStartDragger::quantizeForDisplay(const MidiNoteEvent& note, float timeShiftPixels)
{
    bool snap = sequencer->context->settings()->snapToGrid();
    if (snap) {
        auto scaler = sequencer->context->getScaler();

        float grid = sequencer->context->settings()->getQuarterNotesInGrid();
        float timeShiftMetric = scaler->xToMidiDeltaTime(timeShiftPixels);
        float quantizedMetricTime = TimeUtils::quantizeForEdit(note.startTime, timeShiftMetric, grid);
        float metricDelta = quantizedMetricTime - note.startTime;
        float pixelDelta = scaler->midiTimeTodX(metricDelta);
 #if 0       
        printf("note start = %.2f, pix shift=%.2f unq shift=%.2f, qt=%.2f\n",
            note.startTime,
            timeShiftPixels,
            timeShiftMetric,
            quantizedMetricTime);
        printf("metricDelta = %.2f, finalpixShift = %.2f\n", metricDelta, pixelDelta);
        fflush(stdout);
#endif
          return pixelDelta;
    } else {
        return timeShiftPixels;     // do nothing if off
    }

    // return timeShiftPixels;       // default imp does nothing
  
}

void NoteStartDragger::commit()
{
    auto scaler = sequencer->context->getScaler();
    const float horizontalShiftPix = curMousePositionX - startX; 


    // find the shift required for each note
    std::vector<float> shifts;
    bool isShift = false;
    for (auto it : *sequencer->selection) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(it);
        float timeShiftAmountQuantized = quantizeForDisplay(*note, horizontalShiftPix);
        float timeshiftAmountMetric = scaler->xToMidiDeltaTime(timeShiftAmountQuantized);
        shifts.push_back(timeshiftAmountMetric);
        if (std::abs(timeshiftAmountMetric) > .1) {
            isShift = true;
        }
    }

    if (isShift) {
        sequencer->editor->changeStartTime(shifts);
    }
}

#if 0
void NoteStartDragger::commit()
{
    auto scaler = sequencer->context->getScaler();
    const float horizontalShift = curMousePositionX - startX; 
    const float timeShiftAmount = scaler->xToMidiDeltaTime(horizontalShift);


    // convert quarter notes to 64th notes.
    const int timeShiftTicks = std::round(timeShiftAmount * 16);

    if (timeShiftTicks != 0) {
        sequencer->editor->changeStartTime(true, timeShiftTicks);
    }
}
#endif


/******************************************************************
 *
 * NoteDurationDragger
 */

NoteDurationDragger::NoteDurationDragger(MidiSequencerPtr seq, float x, float y, float duration) :
    NoteHorizontalDragger(seq, x, y, duration)
{
}

void NoteDurationDragger::draw(NVGcontext *vg)
{
    const float horizontalShift = curMousePositionX - startX;
    drawNotes(vg, 0, 0, horizontalShift);
    SqGfx::drawText(vg, curMousePositionX + 20, curMousePositionY + 20, "stretch");
}



void NoteDurationDragger::commit()
{
    auto scaler = sequencer->context->getScaler();
    const float horizontalShift = curMousePositionX - startX;
    const float timeShiftAmount = scaler->xToMidiDeltaTime(horizontalShift);
    const int timeShiftTicks = std::round(timeShiftAmount * 16);
    if (timeShiftTicks != 0) {
        sequencer->editor->changeDuration(true, timeShiftTicks);
    }
}

