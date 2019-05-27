

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
    auto scaler = sequencer->context->getScaler();
    const float verticalShift =  curMousePositionY - startY;
    const float transposeCV = scaler->yToMidiDeltaCVPitch(verticalShift);
    const int semiShift = PitchUtils::deltaCVToSemitone(transposeCV);
    if (semiShift != 0) {
        sequencer->editor->changePitch(semiShift);
    }
}

void NotePitchDragger::draw(NVGcontext *vg)
{
    SqGfx::drawText(vg, curMousePositionX, curMousePositionY, "mouse");
    
    float verticalShift =  curMousePositionY - startY;
    drawNotes(vg, verticalShift, 0, 0);
}

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

