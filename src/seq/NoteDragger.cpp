
#include "NoteDragger.h"



 NoteDragger::NoteDragger(NoteDisplay* display, const Vec& initPos) :
    host(display),
    startPos(initPos)
 {

 }

NoteDragger::~NoteDragger()
{

}


NotePitchDragger::NotePitchDragger(NoteDisplay* display, const Vec& initPos) :
    NoteDragger(display, initPos)
{

}
void NotePitchDragger::onDrag()
{
}


void NotePitchDragger::commit()
{
}