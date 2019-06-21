

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
                UIPrefs::DRAGGED_NOTE_COLOR,
                x, y, width, noteHeight);
        }
    }
}

#if 0
float NoteDragger::getCursorOutsidePitchRange() const
{
    auto scaler = sequencer->context->getScaler();
    assert(scaler);

    const float pitchLow = sequencer->context->pitchLow();
    const float pitchHi = sequencer->context->pitchHi();

    const float mousePitchCV = scaler->yToMidiCVPitch(curMousePositionY);

    float ret = 0;
    if (mousePitchCV > pitchHi) {
        ret = mousePitchCV - pitchHi;
    } else if (mousePitchCV < pitchLow) {
        ret =  mousePitchCV - pitchLow;
    }

    return ret;
}
#endif

/******************************************************************
 *
 * NotePitchDragger 
 */


/*

  const float viewportUpperPitch0;    // The initial pitch of the topmost pixel in the viewport
    const float highPitchForDragStart;  // The pitch at which we start dragging up
    const float viewportLowerPitch0;    // The initial pitch of the bottom most pixel in the viewport
    const float lowPitchForDragStart;   // The pitch at which we start dragging down
 */


// Remember current viewport pitch range. Shave some off top and
// bottom to allow reasonable dragging.
NotePitchDragger::NotePitchDragger(MidiSequencerPtr seq, float x, float y) :
    NoteDragger(seq, x, y),
    viewportUpperPitch0(sequencer->context->pitchHi()),
    highPitchForDragStart(sequencer->context->pitchHi() - 2 * PitchUtils::semitone),
    viewportLowerPitch0(sequencer->context->pitchLow()),
    lowPitchForDragStart(sequencer->context->pitchLow() + 2 * PitchUtils::semitone) 
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
    printf("in calcShift, t=%.2f, hi=%.2f lo=%.2f\n", transpose, highPitchForDragStart, lowPitchForDragStart);
    if (transpose > highPitchForDragStart) {
        ret = transpose - highPitchForDragStart;
    } else if (transpose < lowPitchForDragStart) {
        ret =  transpose - lowPitchForDragStart;
    }

    return ret;
}

void NotePitchDragger::onDrag(float deltaX, float deltaY)
{
    NoteDragger::onDrag(deltaX, deltaY);
    const float transpose = calcTranspose();
    const float shift = calcShift(transpose);

    printf("onDrag, trans = %.2f, shift = %.2f\n", transpose, shift); fflush(stdout);

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
    SqGfx::drawText(vg, curMousePositionX, curMousePositionY, "mouse");
    
    float verticalShift =  curMousePositionY - startY;
    drawNotes(vg, verticalShift, 0, 0);
}




#if 0
void NotePitchDragger::onDrag(float deltaX, float deltaY)
{
    NoteDragger::onDrag(deltaX, deltaY);
    const float pitchShift = getCursorOutsidePitchRange();
   
    if (pitchShift != 0) {
        printf("drag, pitch is %.2f shift is %.2f",  sequencer->context->cursorPitch(), pitchShift);
        #if 0
        float cursorPitch;
        if (pitchShift > 0) {
            cursorPitch = sequencer->context->pitchHi() + pitchShift;
        } else {
            cursorPitch = sequencer->context->pitchLow() + pitchShift;
        }
        #endif
        const float cursorPitch = sequencer->context->cursorPitch() +  pitchShift;
        printf(" -> %.2f \n",  cursorPitch); fflush(stdout);
        
        //float cursorPitch = sequencer->context->cursorPitch();
        //cursorPitch += pitchShift;
        sequencer->context->setCursorPitch(cursorPitch);
        sequencer->context->adjustViewportForCursor();

    }
}
#endif

/******************************************************************
 *
 * NoteStartDragger 
 */
NoteStartDragger::NoteStartDragger(MidiSequencerPtr seq, float x, float y) :
    NoteDragger(seq, x, y)
{ 
}

void NoteStartDragger::draw(NVGcontext *vg)
{
    const float horizontalShift =  curMousePositionX - startX;
    drawNotes(vg, 0, horizontalShift, 0);
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
    NoteDragger(seq, x, y)
{
}

void NoteDurationDragger::draw(NVGcontext *vg)
{
    const float horizontalShift =  curMousePositionX - startX;
    drawNotes(vg, 0, 0, horizontalShift);
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

