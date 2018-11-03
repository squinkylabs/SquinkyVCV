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

    std::vector<MidiEvent> toRem;
    std::vector<MidiEvent> toAdd;

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
    std::vector<MidiEvent> toRem;
    std::vector<MidiEvent> toAdd;

    MidiEvent newEvent;
    newEvent.pitch = 12;
    toAdd.push_back(newEvent);

    CommandPtr cmd = std::make_shared<ReplaceDataCommand>(ms, 0, toRem, toAdd);
    ur->execute(cmd);

    assert(ms->getTrack(0)->size() == 1);     // we added an event
    assert(ur->canUndo());

    auto tv = ms->getTrack(0)->_testGetVector();
    assert(tv[0] == newEvent);

    ur->undo();
    assert(ms->getTrack(0)->size() == 0);     // we added an event
    assert(!ur->canUndo());
}

void testReplaceCommand()
{
    test0();
    test1();
}