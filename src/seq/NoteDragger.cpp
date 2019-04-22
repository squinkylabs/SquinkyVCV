

#include "../Squinky.hpp"
#include "WidgetComposite.h"

#ifdef __V1
#include "widget/Widget.hpp"
#else
#include "widgets.hpp"
#include "util/math.hpp"
#include "window.hpp"
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
  //  printf("x=%.2f y=%.2f\n", curMousePositionX, curMousePositionY); fflush(stdout);
} 


//******************************************************************
//extern int bnd_font;

NotePitchDragger::NotePitchDragger(float x, float y) :
    NoteDragger(x, y)
{
  
    //rintf("in ctor of dragger, font=%d\n", f); fflush(stdout);
}

void NotePitchDragger::commit()
{
}

void NotePitchDragger::draw(NVGcontext *vg)
{
    // move filled rec to a stan-alone class? or repeat it here...
   // host->filledRect(vg,  UIPrefs::NOTE_COLOR, curMousePositionX, curMousePositionY, 10, 10);

#ifdef __V1
    int f = APP->window->uiFont->handle;
#else
    int f = gGuiFont->handle;
#endif
    nvgFillColor(vg, UIPrefs::NOTE_COLOR);
    nvgFontFaceId(vg, f);
    nvgFontSize(vg, 14);
    nvgText(vg, curMousePositionX, curMousePositionY,
        "mouse", nullptr);
}