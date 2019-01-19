
#include "MidiEditorContext.h"


extern int _mdb;

MidiEditorContext::MidiEditorContext()
{
    ++_mdb;
}

MidiEditorContext::~MidiEditorContext()
{
    --_mdb;
}