#include "NewSongDataCommand.h"
#include "MidiLock.h"
#include "MidiSequencer.h"
#include "MidiSong.h"

NewSongDataDataCommand::NewSongDataDataCommand(MidiSongPtr sp, Updater up) :
    newSong(sp),
    updater(up)
{
    name = "Load MIDI file";
}

NewSongDataDataCommand::~NewSongDataDataCommand()
{
}

NewSongDataDataCommandPtr NewSongDataDataCommand::makeLoadMidiFileCommand(
    MidiSongPtr song,
    NewSongDataDataCommand::Updater updater)
{
    return std::make_shared<NewSongDataDataCommand>(song, updater);
}

void NewSongDataDataCommand::execute(MidiSequencerPtr sequencer, SequencerWidget* widget)
{
 
    newSong->assertValid();
    oldSong = sequencer->song;
    fflush(stderr);
    {
        // Must lock the songs when swapping them or player 
        // might glitch (or crash).
        MidiLocker oldL(oldSong->lock);
        MidiLocker newL(newSong->lock);

        // call back to sequencer to swap in the new song
        updater(true, sequencer, newSong, widget);
        //sequencer->setNewSong(newSong);
    }
    // Now that we are outside the scope of the midi lock, update the UI
    updater(false, sequencer, newSong, widget);
    sequencer->assertValid();
}

void NewSongDataDataCommand::undo(MidiSequencerPtr sequencer, SequencerWidget* widget)
{
    oldSong->assertValid();
    newSong->assertValid();
    {
        MidiLocker oldL(oldSong->lock);
        MidiLocker newL(newSong->lock);
        updater(true, sequencer, oldSong, widget);
    }
    updater(false, sequencer, oldSong, widget);
    fflush(stderr);
}