
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

   
    auto updater = [](bool set, MidiSequencerPtr, MidiSongPtr, SequencerWidget*) {

    };
    //  using Updater = std::function<void(MidiSequencerPtr seq, SequencerWidget* widget)>;

    NewSongDataDataCommandPtr cmd = NewSongDataDataCommand::makeLoadMidiFileCommand(song, updater);

    seq->undo->execute(seq, cmd);
    cmd.reset();
}

static void test1()
{
    MidiSongPtr song1 = MidiSong::makeTest(MidiTrack::TestContent::oneQ1, 0);
    MidiSongPtr song2 = MidiSong::makeTest(MidiTrack::TestContent::FourTouchingQuarters, 0);

    assertEQ(song1->getTrack(0)->size(), 2);
    assertEQ(song2->getTrack(0)->size(), 5);

    int updateCount = 0;

    MidiSequencerPtr seq = MidiSequencer::make(
        song1,
        std::make_shared<TestSettings>(),
        std::make_shared<TestAuditionHost>());

    auto updater = [&updateCount](bool set, MidiSequencerPtr seq, MidiSongPtr song, SequencerWidget*) {
        if (set) {
            seq->setNewSong(song);
        }
        if (!set) {
            ++updateCount;
        }
    };

    assertEQ(seq->song->getTrack(0)->size(), 2);

    NewSongDataDataCommandPtr cmd = NewSongDataDataCommand::makeLoadMidiFileCommand(song2, updater);
    assertEQ(updateCount, 0);

    seq->undo->execute(seq, cmd);
    assertEQ(seq->song->getTrack(0)->size(), 5);
    assert(seq->undo->canUndo());
    assertEQ(updateCount, 1);

    seq->undo->undo(seq);
    assertEQ(seq->song->getTrack(0)->size(), 2);
    assert(seq->undo->canRedo());
     assertEQ(updateCount, 2);

    seq->undo->redo(seq);
    assertEQ(seq->song->getTrack(0)->size(), 5);
    assert(seq->undo->canUndo());
     assertEQ(updateCount, 3);

    seq->undo->undo(seq);
    assertEQ(seq->song->getTrack(0)->size(), 2);
     assertEQ(updateCount, 4);
}

void testNewSongDataDataCommand()
{
    assertEvCount(0);
    test0();
    test1();
    assertEvCount(0);
}