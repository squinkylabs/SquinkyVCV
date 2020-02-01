
#include "InteropClipboard.h"
#include "MidiLock.h"
#include "MidiSequencer.h"
#include "SqClipboard.h"
#include "TestAuditionHost.h"
#include "TestSettings.h"

#include <assert.h>

/*********** let's make a test impl of clipboard *****/


static MidiTrackPtr testClipData = nullptr;

bool InteropClipboard::empty()
{
    return !testClipData;
}

MidiTrackPtr InteropClipboard::_getRaw()
{
    return testClipData;
}
void InteropClipboard::put(MidiTrackPtr t, bool selectAll)
{
    auto temp = getCopyData(t, selectAll);
    testClipData = temp;
}
#if 0
void InteropClipboard::put(MidiTrackPtr t, bool selectAll)
{
    t->assertValid();

    // TODO: move this all to common code
    float firstTime = 0;
    float lastTime = 0;
    
    if (t->begin() != t->end()) {
        MidiEventPtr firstEvent = t->begin()->second;
        if (firstEvent->type != MidiEvent::Type::End) {
            firstTime = firstEvent->startTime;
        }
    }

    MidiLockPtr lock= std::make_shared<MidiLock>();
    MidiLocker l(lock);
    testClipData = std::make_shared<MidiTrack>(lock);

    for (auto event : *t) {
        MidiEventPtr clone = event.second->clone();
       // const bool isEnd = clone->type == MidiEvent::Type::End;

        if (!selectAll) {
            clone->startTime -= firstTime;
        }
      

        MidiEndEventPtr end = safe_cast<MidiEndEvent>(clone);
        if (end && !selectAll) {
            end->startTime = lastTime;
        }
        testClipData->insertEvent(clone);

        lastTime = clone->startTime;
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(clone);
        if (note) {
            lastTime += note->duration;
        }

    }
    testClipData->assertValid();
}
#endif

InteropClipboard::PasteData InteropClipboard::get(float insertTime, MidiTrackPtr destTrack, MidiSelectionModelPtr sel)
{
    //return testClipData;
    return getPasteData(insertTime, testClipData, destTrack, sel);
}

void  InteropClipboard::_clear()
{
    testClipData = nullptr;
}

static void testRawClip1()
{
    MidiLockPtr lock = std::make_shared<MidiLock>();
    MidiLocker l(lock);
    MidiTrackPtr track = MidiTrack::makeTest(MidiTrack::TestContent::oneNote123, lock);
    InteropClipboard::put(track, false);

    MidiTrackPtr t2 = InteropClipboard::_getRaw();
    assert(t2);
    t2->assertValid();
    MidiNoteEventPtr note = t2->getFirstNote();
    assertEQ(note->startTime, 0);

    const float len = t2->getLength();
    assertEQ(len, note->startTime + note->duration);
}

static void testRawClip2()
{
    MidiLockPtr lock = std::make_shared<MidiLock>();
    MidiLocker l(lock);
    MidiTrackPtr track = MidiTrack::makeTest(MidiTrack::TestContent::oneNote123, lock);
    InteropClipboard::put(track, true);

    MidiTrackPtr t2 = InteropClipboard::_getRaw();
    assert(t2);
    t2->assertValid();
    MidiNoteEventPtr note = t2->getFirstNote();
    assertEQ(note->startTime, 1.23f);
    assertEQ(t2->getLength(), track->getLength());
}

static void testRawClipPaste(bool selectAll, float insertTime)
{
    // put note on clip
    MidiLockPtr lock = std::make_shared<MidiLock>();
    MidiLocker l(lock);
    MidiTrackPtr track = MidiTrack::makeTest(MidiTrack::TestContent::oneNote123, lock);
    InteropClipboard::put(track, selectAll);

    float rawTime = InteropClipboard::_getRaw()->getFirstNote()->startTime;

    MidiTrackPtr destTrack = MidiTrack::makeTest(MidiTrack::TestContent::empty, lock);
    auto a = std::make_shared<TestAuditionHost>();
    MidiSelectionModelPtr sel = std::make_shared<MidiSelectionModel>(a);
    InteropClipboard::PasteData data = InteropClipboard::get(insertTime, destTrack, sel);

    assert(data.toRemove.empty());
    assert(!data.toAdd.empty());

    auto firstNoteToAdd = data.toAdd[0];
    const float expectedStart = insertTime + (selectAll ? 1.23f : 0.f);
    assertEQ(firstNoteToAdd->startTime, expectedStart)
}

static void testRawClipPaste()
{
    testRawClipPaste(true, 0);
    testRawClipPaste(false, 0);
    testRawClipPaste(true, 1);
    testRawClipPaste(false, 1);
}


static int _trackNumber = 0;

// sequencer factory - helper function
static MidiSequencerPtr makeTest(bool empty = false)
{
    MidiSongPtr song = empty ?
        MidiSong::MidiSong::makeTest(MidiTrack::TestContent::empty, _trackNumber) :
        MidiSong::MidiSong::makeTest(MidiTrack::TestContent::eightQNotes, _trackNumber);
    MidiSequencerPtr sequencer = MidiSequencer::make(
        song,
        std::make_shared<TestSettings>(),
        std::make_shared<TestAuditionHost>());

    sequencer->context->setTrackNumber(_trackNumber);
    sequencer->context->setStartTime(0);
    sequencer->context->setEndTime(
        sequencer->context->startTime() + 8);
    sequencer->context->setPitchLow(PitchUtils::pitchToCV(3, 0));
    sequencer->context->setPitchHi(PitchUtils::pitchToCV(5, 0));

    sequencer->assertValid();
    return sequencer;
}


static void testCopy1()
{
    InteropClipboard::_clear();
    MidiSequencerPtr seq = makeTest(false);
    assertEQ(seq->context->getTrack()->size(), 9);
    seq->editor->selectNextNote();          // now first is selected

    assertEQ(seq->context->getTrack()->getFirstNote()->startTime, 0);

    assert(InteropClipboard::empty());
    seq->editor->copy();

    assertEQ(seq->context->getTrack()->size(), 9);
    auto clip = InteropClipboard::_getRaw();
    assert(clip);
    clip->assertValid();
    assertEQ(clip->size(), 2);       //just the selected note and the end
    assertEQ(clip->getFirstNote()->startTime, 0);
}

static MidiSequencerPtr makeSongPut8NotesOnClip()
{
    InteropClipboard::_clear();
    MidiSequencerPtr seq = makeTest(false);
    assertEQ(seq->context->getTrack()->size(), 9);
    seq->editor->selectAll();
    seq->context->getTrack()->assertValid();
 
    assert(InteropClipboard::empty());
    seq->editor->copy();
    return seq;
}

static void testCopy2()
{
    MidiSequencerPtr seq = makeSongPut8NotesOnClip();
    assertEQ(seq->context->getTrack()->size(), 9);
    auto clip = InteropClipboard::_getRaw();
    assert(clip);
    clip->assertValid();
    assertEQ(clip->size(), 9);       //just the selected note and the end
}

static void testPaste1()
{
    MidiSequencerPtr seq = makeSongPut8NotesOnClip();
    assert(!seq->undo->canUndo());
    seq->selection->clear();   
    seq->editor->paste();
    seq->assertValid();
    assertEQ(seq->context->getTrack()->size(), 9 + 8);
    assert(seq->undo->canUndo());
}

static void testPasteNothingShouldDoNothing()
{
    MidiSequencerPtr seq = makeSongPut8NotesOnClip();
    InteropClipboard::_clear();
    seq->editor->paste();
    assert(!seq->undo->canUndo());
}

static void testPasteOntoSelection()
{
    MidiSequencerPtr seq = makeSongPut8NotesOnClip();
    assert(!seq->undo->canUndo());
    seq->editor->selectAll();
    seq->editor->paste();
    seq->assertValid();

    // pasting a whole track onto itself should leave no change,
    // since it will replace all the existing notes onto themselves.
    assertEQ(seq->context->getTrack()->size(), 9);
    assert(seq->undo->canUndo());
}

static void testPasteTimeSub(float pasteTime)
{
    //assert(false);      // port to new high level clip
    printf("!!! testPasteTImeSub doesn't work\n");
#if 0
    // Make a song with a single note at 1.23
    auto song = MidiSong::MidiSong::makeTest(MidiTrack::TestContent::oneNote123, _trackNumber);
    MidiSequencerPtr seq = MidiSequencer::make(
        song,
        std::make_shared<TestSettings>(),
        std::make_shared<TestAuditionHost>());
    seq->context->setTrackNumber(_trackNumber);
    seq->assertValid();
    MidiLocker l(seq->song->lock);

    MidiNoteEventPtr note = seq->context->getTrack()->getFirstNote();

    // Set the cursor to be on the one note, and select it
    seq->context->setCursorTime(3);     // we don't use cursor here, so set it crazy
    seq->selection->select(note);

    // copy one note at zero-relative time to clip
    seq->editor->copy();

    auto clipTrack = InteropClipboard::get();
    assert(clipTrack);
    assertEQ(clipTrack->size(), 2);
    //float offset = SqClipboard::getTrackData()->offset;
   //assertEQ(offset, 1.23f);
    auto firstNote = clipTrack->getFirstNote();
    assert(firstNote);
    assertEQ(firstNote->startTime, 0);  // time should be relative to first note

    // clear out the track
    seq->context->getTrack()->deleteEvent(*note);
    seq->selection->clear();
    assertEQ(seq->context->getTrack()->size(), 1);

    // at this point there will be notes in the selection that don't exist any more
    seq->assertValid();

    // now paste at pasteTime, with nothing selected in dest
    if (pasteTime >= 0) {
        seq->context->setCursorTime(pasteTime);
        seq->context->adjustViewportForCursor();
        // NOW NEED TO SCROLL TO CURSOR
        seq->selection->clear();
    } else {
        seq->context->setCursorTime(0);
    }

    seq->editor->paste();
    seq->assertValid();

    // note should be at zero time (but not for -1 case, right?
    MidiNoteEventPtr first = seq->context->getTrack()->getFirstNote();
    assert(first);
    if (pasteTime >= 0) {
        assertEQ(first->startTime, pasteTime);
    } else {
        assertEQ(first->startTime, 0);
    }
#endif
}

static void testPasteTime1()
{
    testPasteTimeSub(0);
}

static void testPasteTime2()
{
    testPasteTimeSub(.3f);
}

static void testPasteTime3()
{
    testPasteTimeSub(4567.89f);
}

static void testMidiEditorCCPSub(int tk)
{
    _trackNumber = tk;
    testCopy1();
    testCopy2();

    testPaste1();
    testPasteNothingShouldDoNothing();
    testPasteOntoSelection();
    testPasteTime1();
    testPasteTime2();
    testPasteTime3();
    // need to test pasting one note onto itself
    testPasteTimeSub(-1);
}

static void testRawClip()
{
    testRawClip1();
    testRawClip2();
    testRawClipPaste();
}


void testMidiEditorCCP()
{
    testRawClip();
    testMidiEditorCCPSub(0);
    testMidiEditorCCPSub(2);
    printf("find and fix midi leak\n");
    //assertNoMidi();
}