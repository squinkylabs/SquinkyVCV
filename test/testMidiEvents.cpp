

#include "MidiEvent.h"


static void testType()
{
    MidiNoteEvent note;
    assert(note.type == MidiEvent::Type::Note);
    MidiEndEvent end;
    assert(end.type == MidiEvent::Type::End);
}

static void testCast()
{
    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    MidiEventPtr evn(note);

    MidiEndEventPtr end = std::make_shared<MidiEndEvent>();
    MidiEventPtr eve(end);

    assert(safe_cast<MidiNoteEvent>(evn));
    assert(safe_cast<MidiEndEvent>(eve));

    assert(!safe_cast<MidiNoteEvent>(eve));
    assert(!safe_cast<MidiEndEvent>(evn));

    assert(!safe_cast<MidiNoteEvent>(end));
    assert(!safe_cast<MidiEndEvent>(note));

    assert(safe_cast<MidiEvent>(note));
    assert(safe_cast<MidiEvent>(end));
}

static void testEqual()
{
    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    MidiNoteEventPtr note2 = std::make_shared<MidiNoteEvent>();
    MidiEventPtr evn(note);

    MidiEndEventPtr end = std::make_shared<MidiEndEvent>();
    MidiEventPtr eve(end);

    assertEQ(note, evn);
    assertEQ(end, eve);
    assert (*note == *note2);

    assert(!(*note == *end));
    assert(*note != *end);
    assert(*note != *eve);
    assert(*evn != *end);
    assert(*note != *end);

    note2->pitch = 50;
    assert(*note != *note2);


}

void  testMidiEvents()
{
    testType();
    testCast();

    testEqual();

    printf("*** hey! need == test\n");
 //   assert(false);  // next: operator ==
}