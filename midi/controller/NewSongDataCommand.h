#pragma once


#include "SqCommand.h"

#include <memory>
#include <functional>

class NewSongDataDataCommand;
class MidiSong;

using NewSongDataDataCommandPtr = std::shared_ptr<NewSongDataDataCommand>;
using MidiSongPtr = std::shared_ptr<MidiSong>;

class NewSongDataDataCommand : public SqCommand
{
public:
    // call with set == true to set the song
    // call wiht set == false to update the ui
    using Updater = std::function<void(bool set, MidiSequencerPtr seq, MidiSongPtr song, SequencerWidget* widget)>;

    NewSongDataDataCommand(MidiSongPtr, Updater updater);
    ~NewSongDataDataCommand();
    void execute(MidiSequencerPtr, SequencerWidget*) override;
    void undo(MidiSequencerPtr, SequencerWidget*) override;

    static NewSongDataDataCommandPtr makeLoadMidiFileCommand(
        MidiSongPtr, 
        Updater updater);
private:
    MidiSongPtr newSong;
    MidiSongPtr oldSong;
    Updater updater;
};