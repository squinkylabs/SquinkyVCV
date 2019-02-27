
#include "AboveNoteGrid.h"
#include "MidiSequencer.h"
#include "UIPrefs.h"

AboveNoteGrid::AboveNoteGrid(const Vec& pos, const Vec& size, MidiSequencerPtr seq)
{
    this->box.pos = pos;
    box.size = size;
    sequencer = seq;
}


void AboveNoteGrid::setSequencer(MidiSequencerPtr seq)
{
    sequencer = seq;
}


// TODO: move to util
static void filledRect(NVGcontext *vg, NVGcolor color, float x, float y, float w, float h)
{
    nvgFillColor(vg, color);
    nvgBeginPath(vg);
    nvgRect(vg, x, y, w, h);
    nvgFill(vg);
}


#ifdef __V1
void AboveNoteGrid::draw(const DrawArgs &args)
{
    NVGcontext *vg = args.vg;
#else
void AboveNoteGrid::draw(NVGcontext *vg)
{
#endif
    printf("draw w = %f h = %f\n", box.size.x, box.size.y);
    fflush(stdout);
    filledRect(vg, UIPrefs::NOTE_EDIT_BACKGROUND, 0, 0, box.size.x, box.size.y);
}