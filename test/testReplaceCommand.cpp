#include "asserts.h"

#include "ISeqSettings.h"
#include "MidiEvent.h"
#include "MidiLock.h"
#include "MidiTrack.h"
#include "MidiSequencer.h"
#include "MidiSong.h"
#include "ReplaceDataCommand.h"
#include "TestAuditionHost.h"
#include "TestSettings.h"


// test that functions can be called
static void test0()
{
    MidiSongPtr song = MidiSong::makeTest(MidiTrack::TestContent::empty, 0);
    MidiSequencerPtr seq = MidiSequencer::make(
        song, 
        std::make_shared<TestSettings>(),
        std::make_shared<TestAuditionHost>());

    std::vector<MidiEventPtr> toRem;
    std::vector<MidiEventPtr> toAdd;

    CommandPtr cmd = std::make_shared<ReplaceDataCommand>(song, 0, toRem, toAdd);
    seq->undo->execute(seq, cmd);
}

// Test simple add note command
static void test1()
{
    MidiSongPtr ms = MidiSong::makeTest(MidiTrack::TestContent::empty, 0);
    MidiSequencerPtr seq = MidiSequencer::make(
        ms,
        std::make_shared<TestSettings>(),
        std::make_shared<TestAuditionHost>());

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
    MidiSequencerPtr seq = MidiSequencer::make(ms, std::make_shared<TestSettings>(), std::make_shared<TestAuditionHost>());

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
    MidiSequencerPtr seq = MidiSequencer::make(ms, std::make_shared<TestSettings>(), std::make_shared<TestAuditionHost>());

    MidiEventPtr firstEvent = seq->context->getTrack()->getFirstNote();
    assert(firstEvent);
    seq->selection->select(firstEvent);
    auto cmd = ReplaceDataCommand::makeChangePitchCommand(seq, 1);
    cmd->execute(seq, nullptr);

    firstEvent = seq->context->getTrack()->begin()->second;
    float pitch = safe_cast<MidiNoteEvent>(firstEvent)->pitchCV;

    assertClose(pitch, -.91666, .01);
    seq->assertValid();

    cmd->undo(seq, nullptr);
    seq->assertValid();
    cmd->execute(seq, nullptr);
    seq->assertValid();
}

static void testTrackLength()
{
    MidiSongPtr ms = MidiSong::makeTest(MidiTrack::TestContent::empty, 0);
    MidiLocker l(ms->lock);
    MidiSequencerPtr seq = MidiSequencer::make(ms, std::make_shared<TestSettings>(), std::make_shared<TestAuditionHost>());

    assertEQ(seq->context->getTrack()->size(), 1);     // just an end event
    assertEQ(seq->context->getTrack()->getLength(), 8); // two bars long

    //change to 77 bars
    auto cmd = ReplaceDataCommand::makeMoveEndCommand(seq, 77);
    cmd->execute(seq, nullptr);
    seq->assertValid();

    assertEQ(seq->context->getTrack()->getLength(), 77);

    cmd->undo(seq, nullptr);
    seq->assertValid();
    assertEQ(seq->context->getTrack()->getLength(), 8);

    cmd->execute(seq, nullptr);
    seq->assertValid();
    assertEQ(seq->context->getTrack()->getLength(), 77);

}

// this one shortens the track to less than the time the notes take
static void testTrackLength2()
{
    MidiSongPtr ms = MidiSong::makeTest(MidiTrack::TestContent::FourAlmostTouchingQuarters, 0);
    MidiLocker l(ms->lock);
    MidiSequencerPtr seq = MidiSequencer::make(ms, std::make_shared<TestSettings>(), std::make_shared<TestAuditionHost>());

    assertEQ(seq->context->getTrack()->getLength(), 4); // one bar long

    //change to 1.5 quarter notes
    auto cmd = ReplaceDataCommand::makeMoveEndCommand(seq, 1.5);
    cmd->execute(seq, nullptr);
    seq->assertValid();

    assertEQ(seq->context->getTrack()->getLength(), 1.5);

    cmd->undo(seq, nullptr);
    seq->assertValid();
    assertEQ(seq->context->getTrack()->getLength(), 4);

    cmd->execute(seq, nullptr);
    seq->assertValid();
    assertEQ(seq->context->getTrack()->getLength(), 1.5);
}


static void testInsert()
{
    MidiSongPtr ms = MidiSong::makeTest(MidiTrack::TestContent::empty, 0);
    MidiLocker l(ms->lock);
    MidiSequencerPtr seq = MidiSequencer::make(ms, std::make_shared<TestSettings>(), std::make_shared<TestAuditionHost>());

    assertEQ(seq->context->getTrack()->size(), 1);     // just an end event
    assertEQ(seq->context->getTrack()->getLength(), 8); // two bars long

    const float initLength = seq->context->getTrack()->getLength();

    // let's insert a note way in the future.
    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    note->startTime = 100;
    note->pitchCV = 1.1f;
    note->duration = 2;

    auto cmd = ReplaceDataCommand::makeInsertNoteCommand(seq, note, false);
    cmd->execute(seq, nullptr);
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

    cmd->undo(seq, nullptr);
    assertEQ(initLength, seq->context->getTrack()->getLength());
    seq->assertValid();

    cmd->execute(seq, nullptr);
    assertEQ(longerLength, seq->context->getTrack()->getLength());
    seq->assertValid();
}

static void testStartTimeUnquantized()
{
    // make empty song
    MidiSongPtr ms = MidiSong::makeTest(MidiTrack::TestContent::empty, 0);
    MidiLocker l(ms->lock);
    MidiSequencerPtr seq = MidiSequencer::make(ms, std::make_shared<TestSettings>(), std::make_shared<TestAuditionHost>());

    // put a note into it at time 100;
    auto track = seq->context->getTrack();
    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    note->startTime = 100;
    note->pitchCV = 1.1f;
    note->duration = 2;
    auto cmd = ReplaceDataCommand::makeInsertNoteCommand(seq, note, false);
    cmd->execute(seq, nullptr);
    seq->assertValid();
    assertEQ(seq->selection->size(), 1);

    // shift note later to 1100
    cmd = ReplaceDataCommand::makeChangeStartTimeCommand(seq, 1000.3f, 0);
    seq->undo->execute(seq, cmd);

    seq->assertValid();
    note = track->getFirstNote();   
    assertEQ(note->startTime, 1100.3f);

    seq->undo->undo(seq);
    seq->assertValid();
    note = track->getFirstNote();
    assertEQ(note->startTime, 100.f);

    seq->undo->redo(seq);
    seq->assertValid();
    note = track->getFirstNote();
    assertEQ(note->startTime, 1100.3f);
}


static void testStartTimeQuantized()
{
    const float qGrid = 1;      // quarter notes
    // make empty song
    MidiSongPtr ms = MidiSong::makeTest(MidiTrack::TestContent::empty, 0);
    MidiLocker l(ms->lock);
    MidiSequencerPtr seq = MidiSequencer::make(ms, std::make_shared<TestSettings>(), std::make_shared<TestAuditionHost>());

    // put a note into it at time 100;
    auto track = seq->context->getTrack();
    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    note->startTime = 100;
    note->pitchCV = 1.1f;
    note->duration = 2;
    auto cmd = ReplaceDataCommand::makeInsertNoteCommand(seq, note, false);
    cmd->execute(seq, nullptr);
    seq->assertValid();
    assertEQ(seq->selection->size(), 1);

    // shift note later to 1100
    cmd = ReplaceDataCommand::makeChangeStartTimeCommand(seq, 1000.2f, qGrid);
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
    MidiSequencerPtr seq = MidiSequencer::make(ms, std::make_shared<TestSettings>(), std::make_shared<TestAuditionHost>());

     // put a note into it at time 10, dur 5;
    auto track = seq->context->getTrack();
    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    note->startTime = 10;
    note->pitchCV = 1.1f;
    note->duration = 5;
    auto cmd = ReplaceDataCommand::makeInsertNoteCommand(seq, note, false);
    cmd->execute(seq, nullptr);
    seq->assertValid();

    // now increase dur by 1
    cmd = ReplaceDataCommand::makeChangeDurationCommand(seq, 1.f, false);
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

static void testDurationMulti(bool absoluteDuration)
{
    MidiSongPtr ms = MidiSong::makeTest(MidiTrack::TestContent::empty, 0);
    MidiLocker l(ms->lock);
    MidiSequencerPtr seq = MidiSequencer::make(ms, std::make_shared<TestSettings>(), std::make_shared<TestAuditionHost>());

     // put a note into it at time 10, dur 5;
    auto track = seq->context->getTrack();
    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    note->startTime = 10;
    note->pitchCV = 1.1f;
    note->duration = 5;
    auto cmd = ReplaceDataCommand::makeInsertNoteCommand(seq, note, false);
    cmd->execute(seq, nullptr);
    seq->assertValid();

    // put a second note into it at time 12, dur 2;
    note = std::make_shared<MidiNoteEvent>();
    note->startTime = 11;
    note->pitchCV = 1.1f;
    note->duration = 1;
    cmd = ReplaceDataCommand::makeInsertNoteCommand(seq, note, true);
    cmd->execute(seq, nullptr);
    seq->assertValid();

    assertEQ(seq->selection->size(), 2);

    // now increase dur by 1 (or set to one)
    const float expectedDur1 = absoluteDuration ? 1.f : 6.f;
    const float expectedDur2 = absoluteDuration ? 1.f : 2.f ;
    cmd = ReplaceDataCommand::makeChangeDurationCommand(seq, 1.f, absoluteDuration);
    seq->undo->execute(seq, cmd);
    seq->assertValid();
    note = track->getFirstNote();
    assert(note);
    assertEQ(note->startTime, 10.f);
    assertEQ(note->duration, expectedDur1);

    note = track->getSecondNote();
    assert(note);
    assertEQ(note->startTime, 11.f);
    assertEQ(note->duration, expectedDur2);
}

static void testDurationMulti()
{
    testDurationMulti(false);
    testDurationMulti(true);
}

static void testCut()
{
    MidiSongPtr ms = MidiSong::makeTest(MidiTrack::TestContent::eightQNotes, 0);
    MidiLocker l(ms->lock);
    MidiSequencerPtr seq = MidiSequencer::make(ms, std::make_shared<TestSettings>(), std::make_shared<TestAuditionHost>());

    const float origLen = seq->context->getTrack()->getLength();
    const int origSize = seq->context->getTrack()->size();
    seq->editor->selectAll();
    auto cmd = ReplaceDataCommand::makeDeleteCommand(seq, "cut");

    seq->undo->execute(seq, cmd);
    seq->assertValid();

    assertEQ(1, seq->context->getTrack()->size());
    seq->undo->undo(seq);
    assertEQ(origSize, seq->context->getTrack()->size());
}


static void testNoteFilter()
{
    printf("testNoteFilter\n");
    // test seq starts at 3:0 and goes up in semis
    MidiSongPtr ms = MidiSong::makeTest(MidiTrack::TestContent::eightQNotes, 0);
    MidiSequencerPtr seq = MidiSequencer::make(ms, std::make_shared<TestSettings>(), std::make_shared<TestAuditionHost>());

    MidiEventPtr firstEvent = seq->context->getTrack()->getFirstNote();
    assert(firstEvent);
    seq->selection->select(firstEvent);

    {
        MidiTrack::iterator it = seq->context->getTrack()->begin();
        ++it;
        MidiEventPtr secondEvent = it->second;
        float pitch = safe_cast<MidiNoteEvent>(secondEvent)->pitchCV;
        assertClose(pitch, -1.f + PitchUtils::semitone, .01);
    }

    auto filter = [](MidiEventPtr p) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(p);
        if (note) {
            note->pitchCV = 5.1f;
        }
    };

    auto cmd = ReplaceDataCommand::makeFilterNoteCommand("foo", seq, filter);
    cmd->execute(seq, nullptr);

    firstEvent = seq->context->getTrack()->begin()->second;
    float pitch = safe_cast<MidiNoteEvent>(firstEvent)->pitchCV;
    assertClose(pitch, 5.1f, .01);

    MidiTrack::iterator it = seq->context->getTrack()->begin();
    ++it;
    MidiEventPtr secondEvent = it->second;
    pitch = safe_cast<MidiNoteEvent>(secondEvent)->pitchCV;
    assertClose(pitch, -1.f + PitchUtils::semitone, .01);

    seq->assertValid();

    cmd->undo(seq, nullptr);
    seq->assertValid();
    cmd->execute(seq, nullptr);
    seq->assertValid();
}

static void testReversePitch()
{
    MidiSongPtr ms = MidiSong::makeTest(MidiTrack::TestContent::eightQNotes, 0);
    MidiSequencerPtr seq = MidiSequencer::make(ms, std::make_shared<TestSettings>(), std::make_shared<TestAuditionHost>());

    seq->editor->selectAll();

    const int origSize = seq->context->getTrack()->size();
    MidiNoteEventPtr firstNote = seq->context->getTrack()->getFirstNote();
    MidiNoteEventPtr lastNote = seq->context->getTrack()->getLastNote();

    printf("orig pitches are %.2f %.2f\n", firstNote->pitchCV, lastNote->pitchCV);

    assertGT(lastNote->pitchCV, firstNote->pitchCV + .1);


    auto cmd = ReplaceDataCommand::makeReversePitchCommand(seq);

    cmd->execute(seq, nullptr);
    seq->assertValid();
    assertEQ(seq->context->getTrack()->size(), origSize);

    assertEQ(seq->context->getTrack()->getFirstNote()->pitchCV, lastNote->pitchCV);
    assertEQ(seq->context->getTrack()->getLastNote()->pitchCV, firstNote->pitchCV);

    cmd->undo(seq, nullptr);
    seq->assertValid();
}


static void testChopNotes(float artic)
{
    MidiSongPtr ms = MidiSong::makeTest(MidiTrack::TestContent::oneQ1, 0);
    MidiSequencerPtr seq = MidiSequencer::make(ms, std::make_shared<TestSettings>(), std::make_shared<TestAuditionHost>());

    seq->editor->selectAll();

    const int origSize = seq->context->getTrack()->size();
    assertEQ(origSize, 1+1);

    seq->context->getTrack()->getFirstNote()->pitchCV = 1.3f;
    seq->context->getTrack()->getFirstNote()->duration = artic;
    assertEQ(seq->context->getTrack()->getFirstNote()->startTime, 1);
  //  assertEQ(seq->context->getTrack()->getFirstNote()->duration, 1);

    auto cmd = ReplaceDataCommand::makeChopNoteCommand(seq, 4);
    cmd->execute(seq, nullptr);
    assertEQ(seq->context->getTrack()->size(), 4 + 1);

    const float expectedDur = artic * .25f;
    auto it = seq->context->getTrack()->begin();
    MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(it->second);
    assertEQ(note->startTime, 1);
    assertEQ(note->duration, expectedDur);
    assertEQ(note->pitchCV, 1.3f);

    ++it;
    note = safe_cast<MidiNoteEvent>(it->second);
    assertEQ(note->startTime, 1.25);
    assertEQ(note->duration, expectedDur);
    assertEQ(note->pitchCV, 1.3f);

    ++it;
    note = safe_cast<MidiNoteEvent>(it->second);
    assertEQ(note->startTime, 1.5);
    assertEQ(note->duration, expectedDur);
    assertEQ(note->pitchCV, 1.3f);

    ++it;
    note = safe_cast<MidiNoteEvent>(it->second);
    assertEQ(note->startTime, 1.75);
    assertEQ(note->duration, expectedDur);
    assertEQ(note->pitchCV, 1.3f);

    seq->assertValid();
}

static void testChopNotes()
{
    testChopNotes(1);
    testChopNotes(.85f);
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
    testStartTimeUnquantized();
    testStartTimeQuantized();
    testDuration();
    testDurationMulti();
    testCut();
    testNoteFilter();
    testReversePitch();
    testChopNotes();
}