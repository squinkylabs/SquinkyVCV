
#include "MidiSequencer.h"
#include "MidiEditor.h"
//#include "MidiViewport.h"

int _mdb = 0;       // global instance counter

MidiSequencer::MidiSequencer(std::shared_ptr<MidiSong> sng) :
    selection(std::make_shared<MidiSelectionModel>()),
    song(sng),
    context(std::make_shared<MidiEditorContext>(sng)),
    editor(std::make_shared<MidiEditor>(sng, selection, context)
    )
{
    ++_mdb;
}

MidiSequencer::~MidiSequencer()
{
    --_mdb;
}


void MidiSequencer::assertValid() const
{
    context->assertValid();
    song->assertValid();
}