
#include "NoteDragger.h"



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
void NotePitchDragger::onDrag()
{
}


void NotePitchDragger::commit()
{
}