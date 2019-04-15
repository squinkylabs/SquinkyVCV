

#include "../Squinky.hpp"
#include "WidgetComposite.h"

#ifdef __V1
#include "widget/Widget.hpp"
#else
#include "widgets.hpp"
#include "util/math.hpp"
#endif

#include "NoteDragger.h"
#include "NoteDisplay.h"

#include "UIPrefs.h"



 NoteDragger::NoteDragger(NoteDisplay* display, const sq::Vec& initPos) :
    host(display),
    startPos(initPos)
 {

 }

NoteDragger::~NoteDragger()
{

}


NotePitchDragger::NotePitchDragger(NoteDisplay* display, const sq::Vec& initPos) :
    NoteDragger(display, initPos)
{

}
void NotePitchDragger::onDrag(const sq::Vec& pos)
{
}


void NotePitchDragger::commit()
{
}

void NotePitchDragger::draw(NVGcontext *vg)
{
    //  void filledRect(NVGcontext *vg, NVGcolor color, float x, float y, float w, float h);
    // crazy value - test
    host->filledRect(vg,  UIPrefs::NOTE_COLOR, 10, 10, 10, 10);
}