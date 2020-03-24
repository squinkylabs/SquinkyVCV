

#include "MakeEmptyTrackCommand4.h"
#include "MidiSequencer4.h"
#include "MidiSong4.h"
#include "TimeUtils.h"
#include "UndoRedoStack.h"



/*


std::shared_ptr<Sq4> make(SeqClock::ClockRate rate,
    int numVoices,
    bool toggleStart,
    int trackNum)
{
    assert(numVoices > 0 && numVoices <= 16);

    std::shared_ptr <MidiSong4> song = makeTestSong4(trackNum);

    auto ret = std::make_shared<Sq4>(song);*/

MidiSequencer4Ptr make()
{
    MidiSong4Ptr song = std::make_shared<MidiSong4>();
    MidiSequencer4Ptr seq = MidiSequencer4::make(song);
    assert(seq->undo);
    return seq;
}

static void test1()
{
    MidiSequencer4Ptr seq = make();
    Command4Ptr cmd = MakeEmptyTrackCommand4::createAddTrack(seq, 0, 0, 1);
    assertEQ(cmd->name, "add section");
    cmd->execute(seq, nullptr);
    assert(!seq->undo->canUndo());
    assert(!seq->undo->canRedo());
    seq->undo->execute4(seq, cmd);

    assert(seq->undo->canUndo());
    assert(!seq->undo->canRedo());
    seq->undo->undo4(seq);
}


static void test2()
{
    MidiSequencer4Ptr seq = make();
    Command4Ptr cmd = MakeEmptyTrackCommand4::createAddTrack(seq, 0, 0, TimeUtils::bar2time(2));

    assert(!seq->song->getTrack(0, 0));
   
    // running the command should make a track
    seq->undo->execute4(seq, cmd);
    assert(seq->song->getTrack(0, 0));

    // after undo, track should be gone
    assert(seq->undo->canUndo());
    assert(!seq->undo->canRedo());
    seq->undo->undo4(seq);
    assert(!seq->song->getTrack(0, 0));


    // redo should make the track again
    seq->undo->redo4(seq);
    assert(seq->song->getTrack(0, 0));
}


static void test3()
{
    MidiSequencer4Ptr seq = make();

    // first make a track
    Command4Ptr cmd = MakeEmptyTrackCommand4::createAddTrack(seq, 0, 0, TimeUtils::bar2time(2));
    seq->undo->execute4(seq, cmd);
    assert(seq->song->getTrack(0, 0));

    cmd = MakeEmptyTrackCommand4::createRemoveTrack(seq, 0, 0, TimeUtils::bar2time(2));
    assertEQ(cmd->name, "remove section");
    seq->undo->execute4(seq, cmd);
    assert(!seq->song->getTrack(0, 0));

    seq->undo->undo4(seq);
    assert(seq->song->getTrack(0, 0));

    seq->undo->redo4(seq);
    assert(!seq->song->getTrack(0, 0));

}


void testEditCommands4()
{
    test1();
    test2();
    test3();
}