
#include "NoteDragger.h"



 NoteDragger::NoteDragger(NoteDisplay* display) :
    host(display)
 {

 }

NoteDragger::~NoteDragger()
{

}


NotePitchDragger::NotePitchDragger(NoteDisplay* display) :
    NoteDragger(display)
{

}
void NotePitchDragger::onDrag()
{
}


void NotePitchDragger::commit()
{
}