
#include "MidiSequencer4.h"


MidiSequencer4::MidiSequencer4(MidiSong4Ptr s) : song(s)
{

}

MidiSequencer4Ptr MidiSequencer4::make(MidiSong4Ptr s)
{
    MidiSequencer4Ptr seq(new MidiSequencer4(s));
    return seq;
}