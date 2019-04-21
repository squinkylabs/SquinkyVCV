

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
    curMousePosition = initPos;

}

NoteDragger::~NoteDragger()
{

}

void NoteDragger::onDrag(const sq::Vec& pos)
{
    curMousePosition.x += pos.x;    
    curMousePosition.y += pos.y;
    //printf("x=%.2f y=%.2f\n", curMousePosition.x, curMousePosition.y); fflush(stdout);
} 


//******************************************************************

NotePitchDragger::NotePitchDragger(NoteDisplay* display, const sq::Vec& initPos) :
    NoteDragger(display, initPos)
{

}




void NotePitchDragger::commit()
{
}

void NotePitchDragger::draw(NVGcontext *vg)
{
    host->filledRect(vg,  UIPrefs::NOTE_COLOR, curMousePosition.x, curMousePosition.y, 10, 10);

    nvgText(vg, curMousePosition.x, curMousePosition.y,
        "mouse", nullptr);
}