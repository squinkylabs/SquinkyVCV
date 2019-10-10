
#include "NewSongDataCommand.h"
#include "MidiSequencer.h"
#include "TestAuditionHost.h"
#include "TestSettings.h"

static void test0()
{
    MidiSongPtr song = MidiSong::makeTest(MidiTrack::TestContent::empty, 0);
    MidiSequencerPtr seq = MidiSequencer::make(
        song,
        std::make_shared<TestSettings>(),
        std::make_shared<TestAuditionHost>());

    NewSongDataDataCommandPtr cmd = NewSongDataDataCommand::makeLoadMidiFileCommand();

    seq->undo->execute(seq, cmd);
}

void testNewSongDataDataCommand()
{
    test0();
}