

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



NoteDragger::NoteDragger(float initX, float initY) :
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
    //printf("x=%.2f y=%.2f\n", curMousePosition.x, curMousePosition.y); fflush(stdout);
} 


//******************************************************************

NotePitchDragger::NotePitchDragger(float x, float y) :
    NoteDragger(x, y)
{

}




void NotePitchDragger::commit()
{
}

void NotePitchDragger::draw(NVGcontext *vg)
{
    // move filled rec to a stan-alone class? or repeat it here...
   // host->filledRect(vg,  UIPrefs::NOTE_COLOR, curMousePositionX, curMousePositionY, 10, 10);

    nvgText(vg, curMousePositionX, curMousePositionY,
        "mouse", nullptr);
}