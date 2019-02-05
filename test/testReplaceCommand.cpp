#include "asserts.h"

#include "MidiEvent.h"
#include "MidiTrack.h"
#include "MidiSequencer.h"
#include "MidiSong.h"
#include "ReplaceDataCommand.h"
#include "UndoRedoStack.h"

// test that functions can be called
static void test0()
{
    UndoRedoStackPtr ur(std::make_shared<UndoRedoStack>());
    MidiSongPtr song(std::make_shared<MidiSong>());
    MidiSelectionModelPtr selection = std::make_shared<MidiSelectionModel>();

    std::vector<MidiEventPtr> toRem;
    std::vector<MidiEventPtr> toAdd;

    song->createTrack(0);

    CommandPtr cmd = std::make_shared<ReplaceDataCommand>(song, selection, 0, toRem, toAdd);
    ur->execute(cmd);
}

// Test simple add note command
static void test1()
{
    UndoRedoStackPtr ur(std::make_shared<UndoRedoStack>());
    MidiSongPtr ms(std::make_shared<MidiSong>());
    MidiSelectionModelPtr selection = std::make_shared<MidiSelectionModel>();

    ms->createTrack(0);
    std::vector<MidiEventPtr> toRem;
    std::vector<MidiEventPtr> toAdd;

    MidiNoteEventPtr newNote = std::make_shared<MidiNoteEvent>();
    assert(newNote);
    newNote->pitchCV = 12;
    toAdd.push_back(newNote);

    CommandPtr cmd = std::make_shared<ReplaceDataCommand>(ms, selection, 0, toRem, toAdd);
    ur->execute(cmd);

    assert(ms->getTrack(0)->size() == 1);     // we added an event
    assert(ur->canUndo());

    auto tv = ms->getTrack(0)->_testGetVector();
    assert(*tv[0] == *newNote);

    ur->undo();
    assert(ms->getTrack(0)->size() == 0);     // we added an event
    assert(!ur->canUndo());
    assert(ur->canRedo());
    ur->redo();
    assert(ms->getTrack(0)->size() == 1);
}


// Test simple remove note command
static void test2()
{
    UndoRedoStackPtr ur(std::make_shared<UndoRedoStack>());
    MidiSongPtr ms = MidiSong::makeTest(MidiTrack::TestContent::eightQNotes, 0);
    MidiSelectionModelPtr selection = std::make_shared<MidiSelectionModel>();

    std::vector<MidiEventPtr> toRem;
    std::vector<MidiEventPtr> toAdd;

    MidiTrackPtr track = ms->getTrack(0);
    const int origSize = track->size();
    MidiEventPtr noteToDelete = track->begin()->second;
    assert(noteToDelete);
    toRem.push_back(noteToDelete);

    CommandPtr cmd = std::make_shared<ReplaceDataCommand>(ms, selection, 0, toRem, toAdd);
    ur->execute(cmd);

    assertEQ(ms->getTrack(0)->size(), (origSize - 1));     // we removed an event
    assert(ur->canUndo());

    ur->undo();
    assert(ms->getTrack(0)->size() == origSize);     // we added an event
    assert(!ur->canUndo());
}

static void testTrans()
{
    MidiSongPtr ms = MidiSong::makeTest(MidiTrack::TestContent::eightQNotes, 0);
    MidiSequencerPtr seq = std::make_shared<MidiSequencer>(ms);
    seq->makeEditor();
    seq->assertValid();

    MidiEventPtr firstEvent = seq->context->getTrack()->begin()->second;
    assert(firstEvent);
    seq->selection->select(firstEvent);
    auto cmd = ReplaceDataCommand::makeChangePitchCommand(seq, 1);
    cmd->execute();


    firstEvent = seq->context->getTrack()->begin()->second;
    float pitch = safe_cast<MidiNoteEvent>(firstEvent)->pitchCV;

    assertClose(pitch, -.91666, .01);
    seq->assertValid();

    cmd->undo();
    seq->assertValid();
    cmd->execute();
    seq->assertValid();
}

static void testInsert()
{
    MidiSongPtr ms = MidiSong::makeTest(MidiTrack::TestContent::empty, 0);
    MidiSequencerPtr seq = std::make_shared<MidiSequencer>(ms);
    seq->makeEditor();
    seq->assertValid();

    assertEQ(seq->context->getTrack()->size(), 1);     // just an end event

    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    note->startTime = 100;
    note->pitchCV = 1.1f;
    note->duration = 2;

    auto cmd = ReplaceDataCommand::makeInsertNoteCommand(seq, note);
    cmd->execute();
    seq->assertValid();

    assertEQ(seq->context->getTrack()->size(), 2);
    auto ev = seq->context->getTrack()->begin()->second;
    MidiNoteEventPtrC note2 = safe_cast<MidiNoteEvent>(ev);
    assert(note2);
    assert(*note2 == *note);

    ev = (++seq->context->getTrack()->begin())->second;
    MidiEndEventPtr end = safe_cast<MidiEndEvent>(ev);
    assert(end);

    // quantize to even bars
    int x = (100 + 2) / 4;
    x *= 4;
    assert(x < 102);
    x += 8;     // and round up two bars
    assertEQ(end->startTime, x);

    assert(false);      // undo and redo

}



void testReplaceCommand()
{
    test0();
    test1();
    test2();

    testTrans();
    testInsert();
}