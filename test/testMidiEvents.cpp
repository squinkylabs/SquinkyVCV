

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

void  testMidiEvents()
{
    testType();
    testCast();

    printf("*** hey! need == test\n");
 //   assert(false);  // next: operator ==
}