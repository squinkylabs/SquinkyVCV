
#include "MidiEditorContext.h"
#include "MidiViewport.h"
#include "MidiSong.h"


extern int _mdb;

MidiEditorContext::MidiEditorContext(MidiSongPtr song) : viewport(std::make_shared<MidiViewport>(song))
{
    ++_mdb;
}

MidiEditorContext::~MidiEditorContext()
{
    --_mdb;
}