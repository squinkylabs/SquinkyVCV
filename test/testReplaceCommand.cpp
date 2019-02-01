#include <assert.h>

#include "MidiEvent.h"
#include "MidiTrack.h"
#include "MidiSong.h"
#include "ReplaceDataCommand.h"
#include "UndoRedoStack.h"


// test that functions can be called
static void test0()
{
    UndoRedoStackPtr ur(std::make_shared<UndoRedoStack>());
    MidiSongPtr ms(std::make_shared<MidiSong>());

    std::vector<MidiEventPtr> toRem;
    std::vector<MidiEventPtr> toAdd;

    ms->createTrack(0);

    CommandPtr cmd = std::make_shared<ReplaceDataCommand>(ms, 0, toRem, toAdd);
    ur->execute(cmd);
}

// Test simple add note command
static void test1()
{
    UndoRedoStackPtr ur(std::make_shared<UndoRedoStack>());
    MidiSongPtr ms(std::make_shared<MidiSong>());

    ms->createTrack(0);
    std::vector<MidiEventPtr> toRem;
    std::vector<MidiEventPtr> toAdd;

    MidiNoteEventPtr newNote = std::make_shared<MidiNoteEvent>();
    assert(newNote);
    newNote->pitchCV = 12;
    toAdd.push_back(newNote);

    CommandPtr cmd = std::make_shared<ReplaceDataCommand>(ms, 0, toRem, toAdd);
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
    MidiSongPtr ms = MidiSong::makeTest1();


    std::vector<MidiEventPtr> toRem;
    std::vector<MidiEventPtr> toAdd;

    MidiTrackPtr track = ms->getTrack(0);
    const int origSize = track->size();
    MidiEventPtr noteToDelete = track->begin()->second;
    assert(noteToDelete);
    toRem.push_back(noteToDelete);
    
    CommandPtr cmd = std::make_shared<ReplaceDataCommand>(ms, 0, toRem, toAdd);
    ur->execute(cmd);

    assertEQ(ms->getTrack(0)->size(), (origSize -1));     // we removed an event
    assert(ur->canUndo());

    ur->undo();
    assert(ms->getTrack(0)->size() == origSize);     // we added an event
    assert(!ur->canUndo());

}


void testReplaceCommand()
{
    test0();
    test1();
    test2();
}