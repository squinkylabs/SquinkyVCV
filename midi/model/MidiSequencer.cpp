
#include "MidiSequencer.h"

MidiSequencer::MidiSequencer(std::shared_ptr<MidiSong>) :
    selection( std::make_shared<MidiSelectionModel>())
{

}