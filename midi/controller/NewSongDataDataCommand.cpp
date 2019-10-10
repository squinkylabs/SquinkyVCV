#include "NewSongDataCommand.h"

NewSongDataDataCommandPtr NewSongDataDataCommand::makeLoadMidiFileCommand()
{
    return std::make_shared<NewSongDataDataCommand>();
}

void NewSongDataDataCommand::execute(MidiSequencerPtr)
{

}
void NewSongDataDataCommand::undo(MidiSequencerPtr)
{
    
}