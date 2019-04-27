

#include "../Squinky.hpp"
#include "SqGfx.h"
#include "WidgetComposite.h"

#ifdef __V1
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
    if (deltaX!=0 || deltaY != 0) {
      //  printf("*** dragger::onDragx=%.2f y=%.2f\n", deltaX, deltaY); fflush(stdout);
    }
       // printf("dragger::onDragx=%.2f y=%.2f\n", curMousePositionX, curMousePositionY); fflush(stdout);
} 


void NoteDragger::drawNotes(NVGcontext *vg, float verticalShift, float horizontalShift)
{
   // printf("draw vert=%f h=%f\n", verticalShift, horizontalShift);
    auto scaler = sequencer->context->getScaler();
    assert(scaler);

  //  float verticalShift =  curMousePositionY - startY;
   // printf("vertical pix = %f\n", verticalShift); fflush(stdout);
   // float transposeCV = scaler->yToMidiDeltaCVPitch(verticalShift);

    //std::pair<int, int> x1 = PitchUtils::cvToPitch(transposeCV);
    //float quantizedCV = PitchUtils::pitchToCV(x1.first, x1.second);
    //printf("transpose cv = %f quant=%d\n", transposeCV, quantizedCV);

    // This was lifted from NoteDisplay.
    // Can we refactor and share?
    MidiEditorContext::iterator_pair it = sequencer->context->getEvents();
    
    const int noteHeight = scaler->noteHeight();
    for (; it.first != it.second; ++it.first) {
        auto temp = *(it.first);
        MidiEventPtr evn = temp.second;
        MidiNoteEventPtr ev = safe_cast<MidiNoteEvent>(evn);

        const float x = scaler->midiTimeToX(*ev) + horizontalShift;
        const float y = scaler->midiPitchToY(*ev) + verticalShift;
        const float width = scaler->midiTimeTodX(ev->duration);

        const bool selected = sequencer->selection->isSelected(ev);
        if (selected) {
          //  printf("drawing gdragged at %.2f, %.2f\n ", x, y);
            SqGfx::filledRect(
                vg,
                UIPrefs::DRAGGED_NOTE_COLOR,
                x, y, width, noteHeight);
        }
    }
}



/******************************************************************
 *
 * NotePitchDragger 
 */

NotePitchDragger::NotePitchDragger(MidiSequencerPtr seq, float x, float y) :
    NoteDragger(seq, x, y)
{
}

void NotePitchDragger::commit()
{
   // printf("NotePitchDragger::commit\n"); fflush(stdout);
    auto scaler = sequencer->context->getScaler();
    float verticalShift =  curMousePositionY - startY;
    float transposeCV = scaler->yToMidiDeltaCVPitch(verticalShift);

    int semiShift = PitchUtils::deltaCVToSemitone(transposeCV);
#if 0
    printf("will shift by %d semis\n", semiShift);
    printf("selection has %d notes\n", sequencer->selection->size());
     fflush(stdout);
#endif
    if (semiShift != 0) {
        sequencer->editor->changePitch(semiShift);
    }
}

void NotePitchDragger::draw(NVGcontext *vg)
{
    SqGfx::drawText(vg, curMousePositionX, curMousePositionY, "mouse");
    
    float verticalShift =  curMousePositionY - startY;
    drawNotes(vg, verticalShift, 0);
}

#if 0
void NotePitchDragger::drawNotes(NVGcontext *vg)
{
    auto scaler = sequencer->context->getScaler();
    assert(scaler);

    float verticalShift =  curMousePositionY - startY;
   // printf("vertical pix = %f\n", verticalShift); fflush(stdout);
   // float transposeCV = scaler->yToMidiDeltaCVPitch(verticalShift);

    //std::pair<int, int> x1 = PitchUtils::cvToPitch(transposeCV);
    //float quantizedCV = PitchUtils::pitchToCV(x1.first, x1.second);
    //printf("transpose cv = %f quant=%d\n", transposeCV, quantizedCV);

    // This was lifted from NoteDisplay.
    // Can we refactor and share?
    MidiEditorContext::iterator_pair it = sequencer->context->getEvents();
    
    const int noteHeight = scaler->noteHeight();
    for (; it.first != it.second; ++it.first) {
        auto temp = *(it.first);
        MidiEventPtr evn = temp.second;
        MidiNoteEventPtr ev = safe_cast<MidiNoteEvent>(evn);

        const float x = scaler->midiTimeToX(*ev);
        const float y = scaler->midiPitchToY(*ev) + verticalShift;
        const float width = scaler->midiTimeTodX(ev->duration);

        const bool selected = sequencer->selection->isSelected(ev);
        if (selected) {
          //  printf("drawing gdragged at %.2f, %.2f\n ", x, y);
            SqGfx::filledRect(
                vg,
                UIPrefs::DRAGGED_NOTE_COLOR,
                x, y, width, noteHeight);
        }
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
    drawNotes(vg, 0, horizontalShift);
}

void NoteStartDragger::commit()
{
    printf("NoteStartDragger::commit\n"); fflush(stdout);
    auto scaler = sequencer->context->getScaler();
    const float horizontalShift =  curMousePositionX - startX;
    float timeShiftAmount = scaler->xToMidiDeltaTime(horizontalShift);

    // convert qusrter notes to 64th notes.
    int timeShiftTicks = std::round(timeShiftAmount * 16);

//void changeStartTime(bool ticks, int amount);
//   int semiShift = PitchUtils::deltaCVToSemitone(transposeCV);
#if 0
    printf("will shift by %f pix, %d in midi time\n", 
        horizontalShift,timeShiftAmount);

    printf("selection has %d notes\n", sequencer->selection->size());
     fflush(stdout);
#endif
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
}

void NoteDurationDragger::commit()
{
}

