#include "asserts.h"

#include "ISeqSettings.h"
#include "MidiEvent.h"
#include "MidiLock.h"
#include "MidiTrack.h"
#include "MidiSequencer.h"
#include "MidiSong.h"
#include "ReplaceDataCommand.h"
#include "TestSettings.h"


// test that functions can be called
static void test0()
{
    MidiSongPtr song = MidiSong::makeTest(MidiTrack::TestContent::empty, 0);
    MidiSequencerPtr seq = MidiSequencer::make(song, std::make_shared<TestSettings>());

    std::vector<MidiEventPtr> toRem;
    std::vector<MidiEventPtr> toAdd;

    CommandPtr cmd = std::make_shared<ReplaceDataCommand>(song, 0, toRem, toAdd);
    seq->undo->execute(seq, cmd);
}

// Test simple add note command
static void test1()
{
    MidiSongPtr ms = MidiSong::makeTest(MidiTrack::TestContent::empty, 0);
    MidiSequencerPtr seq = MidiSequencer::make(ms, std::make_shared<TestSettings>());

    std::vector<MidiEventPtr> toRem;
    std::vector<MidiEventPtr> toAdd;

    assert(ms->getTrack(0)->size() == 1);           // just end event
    MidiNoteEventPtr newNote = std::make_shared<MidiNoteEvent>();
    assert(newNote);
    newNote->pitchCV = 12;
    toAdd.push_back(newNote);

    CommandPtr cmd = std::make_shared<ReplaceDataCommand>(ms, 0, toRem, toAdd);
    seq->undo->execute(seq, cmd);

    assertEQ(ms->getTrack(0)->size(), 2);     // we added an event
    assert(seq->undo->canUndo());

    auto tv = ms->getTrack(0)->_testGetVector();
    assert(*tv[0] == *newNote);

    seq->undo->undo(seq);
    assertEQ(ms->getTrack(0)->size(), 1);     // back to just end
    assert(!seq->undo->canUndo());
    assert(seq->undo->canRedo());
    seq->undo->redo(seq);
    assertEQ(ms->getTrack(0)->size(), 2);
}


// Test simple remove note command
static void test2()
{
    MidiSongPtr ms = MidiSong::makeTest(MidiTrack::TestContent::eightQNotes, 0);
    MidiSequencerPtr seq = MidiSequencer::make(ms, std::make_shared<TestSettings>());

    std::vector<MidiEventPtr> toRem;
    std::vector<MidiEventPtr> toAdd;

    MidiTrackPtr track = ms->getTrack(0);
    const int origSize = track->size();
    auto noteToDelete = track->getFirstNote();
    toRem.push_back(noteToDelete);

    CommandPtr cmd = std::make_shared<ReplaceDataCommand>(ms, 0, toRem, toAdd);
    seq->undo->execute(seq, cmd);

    assertEQ(ms->getTrack(0)->size(), (origSize - 1));     // we removed an event
    assert(seq->undo->canUndo());

    seq->undo->undo(seq);
    assert(ms->getTrack(0)->size() == origSize);     // we added an event
    assert(!seq->undo->canUndo());
}

static void testTrans()
{
    MidiSongPtr ms = MidiSong::makeTest(MidiTrack::TestContent::eightQNotes, 0);
    MidiSequencerPtr seq = MidiSequencer::make(ms, std::make_shared<TestSettings>());

    MidiEventPtr firstEvent = seq->context->getTrack()->getFirstNote();
    assert(firstEvent);
    seq->selection->select(firstEvent);
    auto cmd = ReplaceDataCommand::makeChangePitchCommand(seq, 1);
    cmd->execute(seq);

    firstEvent = seq->context->getTrack()->begin()->second;
    float pitch = safe_cast<MidiNoteEvent>(firstEvent)->pitchCV;

    assertClose(pitch, -.91666, .01);
    seq->assertValid();

    cmd->undo(seq);
    seq->assertValid();
    cmd->execute(seq);
    seq->assertValid();
}

static void testTrackLength()
{
    MidiSongPtr ms = MidiSong::makeTest(MidiTrack::TestContent::empty, 0);
    MidiLocker l(ms->lock);
    MidiSequencerPtr seq = MidiSequencer::make(ms, std::make_shared<TestSettings>());

    assertEQ(seq->context->getTrack()->size(), 1);     // just an end event
    assertEQ(seq->context->getTrack()->getLength(), 8); // two bars long

    //change to 77 bars
    auto cmd = ReplaceDataCommand::makeMoveEndCommand(seq, 77);
    cmd->execute(seq);
    seq->assertValid();

    assertEQ(seq->context->getTrack()->getLength(), 77);

    cmd->undo(seq);
    seq->assertValid();
    assertEQ(seq->context->getTrack()->getLength(), 8);

    cmd->execute(seq);
    seq->assertValid();
    assertEQ(seq->context->getTrack()->getLength(), 77);

}

// this one shortens the track to less than the time the notes take
static void testTrackLength2()
{
    MidiSongPtr ms = MidiSong::makeTest(MidiTrack::TestContent::FourAlmostTouchingQuarters, 0);
    MidiLocker l(ms->lock);
    MidiSequencerPtr seq = MidiSequencer::make(ms, std::make_shared<TestSettings>());

    assertEQ(seq->context->getTrack()->getLength(), 4); // one bar long

    //change to 1.5 quarter notes
    auto cmd = ReplaceDataCommand::makeMoveEndCommand(seq, 1.5);
    cmd->execute(seq);
    seq->assertValid();

    assertEQ(seq->context->getTrack()->getLength(), 1.5);

    cmd->undo(seq);
    seq->assertValid();
    assertEQ(seq->context->getTrack()->getLength(), 4);

    cmd->execute(seq);
    seq->assertValid();
    assertEQ(seq->context->getTrack()->getLength(), 1.5);
}


static void testInsert()
{
    MidiSongPtr ms = MidiSong::makeTest(MidiTrack::TestContent::empty, 0);
    MidiLocker l(ms->lock);
    MidiSequencerPtr seq = MidiSequencer::make(ms, std::make_shared<TestSettings>());

    assertEQ(seq->context->getTrack()->size(), 1);     // just an end event
    assertEQ(seq->context->getTrack()->getLength(), 8); // two bars long

    const float initLength = seq->context->getTrack()->getLength();

    // let's insert a note way in the future.
    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    note->startTime = 100;
    note->pitchCV = 1.1f;
    note->duration = 2;

    auto cmd = ReplaceDataCommand::makeInsertNoteCommand(seq, note);
    cmd->execute(seq);
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
    x += 4;     // and round up one bar
    assertEQ(end->startTime, x);

    const float longerLength = seq->context->getTrack()->getLength();

    seq->assertValid();

    cmd->undo(seq);
    assertEQ(initLength, seq->context->getTrack()->getLength());
    seq->assertValid();

    cmd->execute(seq);
    assertEQ(longerLength, seq->context->getTrack()->getLength());
    seq->assertValid();
}

static void testStartTime()
{
    // make empty song
    MidiSongPtr ms = MidiSong::makeTest(MidiTrack::TestContent::empty, 0);
    MidiLocker l(ms->lock);
    MidiSequencerPtr seq = MidiSequencer::make(ms, std::make_shared<TestSettings>());

    // put a note into it at time 100;
    auto track = seq->context->getTrack();
    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    note->startTime = 100;
    note->pitchCV = 1.1f;
    note->duration = 2;
    auto cmd = ReplaceDataCommand::makeInsertNoteCommand(seq, note);
    cmd->execute(seq);
    seq->assertValid();
    assertEQ(seq->selection->size(), 1);

    // shift note later to 1100
    cmd = ReplaceDataCommand::makeChangeStartTimeCommand(seq, 1000.f);
    seq->undo->execute(seq, cmd);

    seq->assertValid();
    note = track->getFirstNote();   
    assertEQ(note->startTime, 1100.f);

    seq->undo->undo(seq);
    seq->assertValid();
    note = track->getFirstNote();
    assertEQ(note->startTime, 100.f);

    seq->undo->redo(seq);
    seq->assertValid();
    note = track->getFirstNote();
    assertEQ(note->startTime, 1100.f);
}

static void testDuration()
{
    MidiSongPtr ms = MidiSong::makeTest(MidiTrack::TestContent::empty, 0);
    MidiLocker l(ms->lock);
    MidiSequencerPtr seq = MidiSequencer::make(ms, std::make_shared<TestSettings>());

     // put a note into it at time 10, dur 5;
    auto track = seq->context->getTrack();
    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    note->startTime = 10;
    note->pitchCV = 1.1f;
    note->duration = 5;
    auto cmd = ReplaceDataCommand::makeInsertNoteCommand(seq, note);
    cmd->execute(seq);
    seq->assertValid();

    // now increase dur by 1
    cmd = ReplaceDataCommand::makeChangeDurationCommand(seq, 1.f);
    seq->undo->execute(seq, cmd);
    seq->assertValid();
    note = track->getFirstNote();
    assert(note);

    assertEQ(note->startTime, 10.f);
    assertEQ(note->duration, 6.f)

    seq->undo->undo(seq);
    seq->assertValid();
    note = track->getFirstNote();
    assertEQ(note->duration, 5.f);

    seq->undo->redo(seq);
    seq->assertValid();
    note = track->getFirstNote();
    assertEQ(note->duration, 6.f);
}

void testReplaceCommand()
{
    test0();
    test1();
    test2();

    testTrans();
    testTrackLength();
    testTrackLength2();
    testInsert();
    testStartTime();
    testDuration();
}