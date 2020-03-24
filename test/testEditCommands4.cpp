

#include "MakeEmptyTrackCommand4.h"
#include "MidiSequencer4.h"
#include "MidiSong4.h"
#include "UndoRedoStack.h"

static void test0()
{
    Command4Ptr cmd = MakeEmptyTrackCommand4::create();
}

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
    Command4Ptr cmd = MakeEmptyTrackCommand4::create();
    cmd->execute(seq, nullptr);
    assert(!seq->undo->canUndo());
    assert(!seq->undo->canRedo());
    seq->undo->execute(seq, cmd);

    assert(seq->undo->canUndo());
    assert(!seq->undo->canRedo());
    seq->undo->execute(seq, cmd);
}

void testEditCommands4()
{
    test0();
    test1();
}