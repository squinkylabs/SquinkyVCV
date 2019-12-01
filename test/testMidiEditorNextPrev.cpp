
#include "asserts.h"
#include "MidiLock.h"
#include "MidiSequencer.h"
#include "TestAuditionHost.h"
#include "TestSettings.h"

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

static bool cursorOnSelection(MidiSequencerPtr seq)
{
    if (seq->selection->empty()) {
        return true;
    }

    assert(seq->selection->size() == 1);    // haven't done multi yet
    MidiEventPtr sel = *seq->selection->begin();
    MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(sel);
    assert(note);

    // for now, do exact match
    if ((note->startTime == seq->context->cursorTime()) &&
        (note->pitchCV == seq->context->cursorPitch())) {
        return true;
    }
    return false;
}


// from a null selection, select next
// should select first after cursor time 0
static void testNextInitialState()
{
    MidiSequencerPtr seq = makeTest();
    seq->editor->selectNextNote();
    assertEQ(seq->selection->size(), 1);     // should be one note selected

    // note should be first one
    MidiEventPtr firstEvent = seq->context->getTrack()->begin()->second;
    assert(seq->selection->isSelected(firstEvent));
    assert(cursorOnSelection(seq));
}

// from a null selection, select previous. should select first
static void testPrevInitialState()
{
    MidiSequencerPtr seq = makeTest();
    seq->editor->selectPrevNote();
    assertEQ(seq->selection->size(), 1);     // should be one note selected

    // note should be first one
    MidiEventPtr firstEvent = seq->context->getTrack()->begin()->second;
    assert(seq->selection->isSelected(firstEvent));
    assert(cursorOnSelection(seq));
}

// from nothing selected, but not at zero. should select the next note.
static void testNextNoSelectionNonZeroCursor()
{
    MidiSequencerPtr seq = makeTest();
    seq->context->setCursorTime(.5);        // set an eight note into it
    seq->editor->selectNextNote();
    assertEQ(seq->selection->size(), 1);     // should be one note selected

    // note should be second one
   // MidiEventPtr firstEvent = seq->context->getTrack()->begin()->second;
   // MidiEventPtr secondEvent =
    auto it = seq->context->getTrack()->begin();
    ++it;
    MidiEventPtr secondEvent = it->second;
    assert(seq->selection->isSelected(secondEvent));
    assert(cursorOnSelection(seq));
}

// from nothing selected, but not at zero. should select the prev note.
static void testPrevNoSelectionNonZeroCursor()
{
    MidiSequencerPtr seq = makeTest();
    seq->context->setCursorTime(1.5);        // set a quarter and an eight note into it 
                                            // (in between second and third note)
    seq->editor->selectPrevNote();
    assertEQ(seq->selection->size(), 1);     // should be one note selected

    auto it = seq->context->getTrack()->begin();
    ++it;
    MidiEventPtr secondEvent = it->second;
    assert(seq->selection->isSelected(secondEvent));
    assert(cursorOnSelection(seq));
}

static void testPrevWhenPastAll()
{
    MidiSequencerPtr seq = makeTest();
    seq->context->setCursorTime(123);        // way past all the notes
                                            // (in between second and third note)
    seq->context->adjustViewportForCursor();

    seq->editor->selectPrevNote();
    assertEQ(seq->selection->size(), 1);     // should be one note selected
}

// with last note selected, should not do anything
static void testNextLastNoteSelected()
{
    MidiSequencerPtr seq = makeTest();
    seq->context->setCursorTime(.5);        // set an eight note into it

    // last note should stay selected
    for (int i = 0; i < 10; ++i) {
        seq->editor->selectNextNote();
        assertEQ(seq->selection->size(), 1);     // should be one note selected
    }

    MidiEventPtr last = *seq->selection->begin();
    MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(last);
    assert(note);
    auto pitch = note->getPitch();
    assertEQ(pitch.second, 7);
}

static void testNext1e()
{
    MidiSongPtr song = std::make_shared<MidiSong>();
    song->createTrack(0);
    MidiTrackPtr track = song->getTrack(0);
    auto lock =  track->lock;

  // make two notes at start time, and one after
    MidiLocker l(lock);
    MidiNoteEventPtr ev1 = std::make_shared<MidiNoteEvent>();
    ev1->startTime = 0;
    ev1->pitchCV = 0;
    track->insertEvent(ev1);

    MidiNoteEventPtr ev2 = std::make_shared<MidiNoteEvent>();
    ev2->startTime = 0;
    ev2->pitchCV = 1;
    track->insertEvent(ev2);

    MidiNoteEventPtr ev3 = std::make_shared<MidiNoteEvent>();
    ev3->startTime = 100;
    ev3->pitchCV = .5f;
    track->insertEvent(ev3);

    MidiEndEventPtr ev4 = std::make_shared<MidiEndEvent>();
    ev4->startTime = 200;
    track->insertEvent(ev4);

    MidiSequencerPtr seq = MidiSequencer::make(
        song,
        std::make_shared<TestSettings>(),
        std::make_shared<TestAuditionHost>());

    // select first event
    seq->editor->selectNextNote();
    assertEQ(seq->selection->size(), 1);     // should be one note selected
    assert(*seq->selection->begin() == ev1);

    seq->editor->selectNextNote();
    assertEQ(seq->selection->size(), 1);     // should be one note selected
    assert(*seq->selection->begin() == ev2);

    seq->editor->selectNextNote();
    assertEQ(seq->selection->size(), 1);     // should be one note selected
    assert(*seq->selection->begin() == ev3);
 
}

static void testNextPrevSingleNote()
{
    MidiSongPtr song = std::make_shared<MidiSong>();
    song->createTrack(0);
    MidiTrackPtr track = song->getTrack(0);
    auto lock = track->lock;


  // make two notes at start time, and one after
    MidiLocker l(lock);
    MidiNoteEventPtr ev1 = std::make_shared<MidiNoteEvent>();
    ev1->startTime = 2.3f;
    ev1->pitchCV = 0;
    track->insertEvent(ev1);

    MidiEndEventPtr ev2 = std::make_shared<MidiEndEvent>();
    ev2->startTime = 4;
    track->insertEvent(ev2);

    MidiSequencerPtr seq = MidiSequencer::make(song, std::make_shared<TestSettings>(), std::make_shared<TestAuditionHost>());
    auto x = seq->context->cursorTime();
    seq->context->adjustViewportForCursor();
    x = seq->context->cursorTime();
    seq->assertValid();

    printf("\n-- about to go to next\n");
    // select first event
    seq->editor->selectNextNote();
    seq->assertValid();
    assertEQ(seq->selection->size(), 1);     // should be one note selected
    
    seq->editor->selectNextNote();
    seq->assertValid();
    assertEQ(seq->selection->size(), 1);     // should be one note selected
    
    seq->editor->selectPrevNote();
    seq->assertValid();
    assertEQ(seq->selection->size(), 1);     // should be one note selected
    
    seq->editor->selectPrevNote();
    seq->assertValid();
    assertEQ(seq->selection->size(), 1);     // should be one note selected
}


// from a non-null selection, select next
static void testNextWhenFirstIsSelected()
{
    MidiSequencerPtr seq = makeTest();
    seq->editor->selectNextNote();
    assertEQ(seq->selection->size(), 1);     // should be one note selected

    MidiEventPtr firstEvent = seq->context->getTrack()->begin()->second;
    assert(seq->selection->isSelected(firstEvent));

    // Above is just test1, so now first event selected
    seq->editor->selectNextNote();
    assertEQ(seq->selection->size(), 1);     // should be one note selected

    auto iter = seq->context->getTrack()->begin();
    ++iter;
    MidiEventPtr secondEvent = iter->second;
    assert(seq->selection->isSelected(secondEvent));
}

static void textExtendNoteWhenOneSelected()
{
    MidiSequencerPtr seq = makeTest();
    seq->editor->selectNextNote();
    assertEQ(seq->selection->size(), 1);  
    seq->editor->extendSelectionToNextNote();
    assertEQ(seq->selection->size(), 2); 
}

static void textExtendNoteTwiceWhenOneSelected()
{
    MidiSequencerPtr seq = makeTest();
    seq->editor->selectNextNote();
    assertEQ(seq->selection->size(), 1);
    assertClose(seq->context->cursorTime(), 0, .001);

    seq->editor->extendSelectionToNextNote();
    assertEQ(seq->selection->size(), 2);
    assertClose(seq->context->cursorTime(), 1, .001);

    seq->editor->extendSelectionToNextNote();
    assertEQ(seq->selection->size(), 3);
}

static void textExtendNotePrevWhenOneSelected()
{
    MidiSequencerPtr seq = makeTest();

    // select the second note
    seq->editor->selectNextNote();
    seq->editor->selectNextNote();
    assertEQ(seq->selection->size(), 1);
    assertClose(seq->context->cursorTime(), 1, .001);

    seq->editor->extendSelectionToPrevNote();
    assertEQ(seq->selection->size(), 2);
    assertClose(seq->context->cursorTime(), 0, .001);
}

static void textExtendNoteTwicePrevWhenOneSelected()
{
    MidiSequencerPtr seq = makeTest();

    // select the third note
    seq->editor->selectNextNote();
    seq->editor->selectNextNote();
    seq->editor->selectNextNote();
    assertEQ(seq->selection->size(), 1);
    assertClose(seq->context->cursorTime(), 2, .001);

    seq->editor->extendSelectionToPrevNote();
    assertEQ(seq->selection->size(), 2);
    assertClose(seq->context->cursorTime(), 1, .001);

    seq->editor->extendSelectionToPrevNote();
    assertEQ(seq->selection->size(), 3);
    assertClose(seq->context->cursorTime(), 0, .001);
}


// from a non-null selection, select previous
static void testPrevWhenSecondSelected()
{
    MidiSequencerPtr seq = makeTest();

    // Select the second note in the Seq
    seq->editor->selectNextNote();
    seq->editor->selectNextNote();
    assertEQ(seq->selection->size(), 1);

    // Verify that second note is selected
    auto iter = seq->context->getTrack()->begin();
    ++iter;
    MidiEventPtr secondEvent = iter->second;
    assert(seq->selection->isSelected(secondEvent));

    // Above is just test1, so now second event selected
    seq->editor->selectPrevNote();
    assertEQ(seq->selection->size(), 1);     // should be one note selected

    iter = seq->context->getTrack()->begin();
    MidiEventPtr firstEvent = iter->second;
    assert(seq->selection->isSelected(firstEvent));
}

static void testPrevWhenFirstSelected()
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->selectNextNote();          // now first is selected
    assert(!seq->selection->empty());
    seq->editor->assertCursorInSelection();

    seq->editor->selectPrevNote();
    assert(!seq->selection->empty());
    auto iter = seq->context->getTrack()->begin();
    MidiEventPtr firstEvent = iter->second;
    assert(seq->selection->isSelected(firstEvent));
}

// from a null selection, select next in null track
static void testNextInEmptyTrack()
{
    MidiSequencerPtr seq = makeTest(true);
    seq->editor->selectNextNote();
    assertEQ(seq->selection->size(), 0);     // should be nothing selected
    assert(seq->selection->empty());
}

// from a null selection, select previous in null track
static void testPrevInEmptyTrack()
{
    MidiSequencerPtr seq = makeTest(true);
    seq->editor->selectPrevNote();
    assertEQ(seq->selection->size(), 0);     // should be nothing selected
    assert(seq->selection->empty());
}

#if 0 // not valid any more
// select one after another until end
static void testNext4()
{
    MidiSequencerPtr seq = makeTest();
    int notes = 0;
    for (bool done = false; !done; ) {
        seq->editor->selectNextNote();
        if (seq->selection->empty()) {
            done = true;
        } else {
            ++notes;
        }
    }
    assertEQ(notes, 8);
}
#endif

// select next that off way out of viewport
static void testNextWhenOutsideViewport()
{
    // Select the first note, and move it somewhere crash
    MidiSequencerPtr seq = makeTest();
    seq->editor->selectNextNote();
    assertEQ(seq->selection->size(), 1);

    // Give the (first) note a pitch and start time that are
    // way outside viewport
    seq->editor->changePitch(50);
    seq->editor->changeStartTime(false, 50);

    assertEQ(seq->selection->size(), 1);
    seq->assertValid();
    seq->editor->assertCursorInSelection();

    seq->editor->selectPrevNote();
    assertEQ(seq->selection->size(), 1);
    seq->assertValid();
    seq->editor->assertCursorInSelection();

    seq->editor->selectNextNote();
    assertEQ(seq->selection->size(), 1);
    seq->assertValid();
    seq->editor->assertCursorInSelection();
}

static void testPrevWhenBeforeStart()
{
    MidiSequencerPtr seq = makeTest(true);
    assertEQ (seq->context->getTrack()->size(), 1);

    // add a single note a time 1
    seq->context->setCursorTime(1);
    
    seq->context->insertNoteDuration = .25f;
    seq->editor->insertDefaultNote(false, false);
    assertEQ(seq->context->getTrack()->size(), 2);

    seq->context->setCursorTime(0);             // cursor before all notes
    seq->selection->clear();                    // and of course nothing selected

    seq->editor->selectPrevNote(); 
    assertEQ(seq->selection->size(), 0);
    assertEQ(seq->context->cursorTime(), 0);
}



void testMidiEditorNextPrevSub(int trackNumber)
{
    _trackNumber = trackNumber;
    testNextInitialState();
    testPrevInitialState();
    testNextNoSelectionNonZeroCursor();
    testPrevNoSelectionNonZeroCursor();
    testNextLastNoteSelected();
    testNextPrevSingleNote();

// for later with fancy multi-select
 //   testNext1e();
//    printf("can pass testNext1e yet\n");
    testNextWhenFirstIsSelected();
    testPrevWhenFirstSelected();
    testPrevWhenSecondSelected();

    testPrevWhenPastAll();

    testNextInEmptyTrack();
    testPrevInEmptyTrack();

#ifdef _NEWTAB
    printf("with new alg, what should testNextWhenOutsideViewport do?\n");
#else
    testNextWhenOutsideViewport();
#endif

    textExtendNoteWhenOneSelected();
}

void testMidiEditorNextPrev()
{
    testMidiEditorNextPrevSub(0);
    testMidiEditorNextPrevSub(2);

    // ok, that's enough testing of multi tracks.
    _trackNumber = 3;
    textExtendNoteTwiceWhenOneSelected();
    textExtendNotePrevWhenOneSelected();
    textExtendNoteTwicePrevWhenOneSelected();

    testPrevWhenBeforeStart();
}