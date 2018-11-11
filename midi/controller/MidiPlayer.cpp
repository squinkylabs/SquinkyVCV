
#include "MidiPlayer.h"

void MidiPlayer::timeElapsed(float seconds)
{
  //  curAbsTime += seconds;
    curMetricTime += seconds * 120.0f / 60.0f;        // fixed at 120 bpm for now

    // when we do note, off, will need to look at note off times, too.
    while (curEvent->first <= curMetricTime) {
        MidiEventPtr event = curEvent->second;
        switch (event->type) {
            case MidiEvent::Type::Note:
                {
                    MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(event);
                    // should now output the note.
                    host->setCV(note->pitch);
                    host->setGate(true);
                    // and save off the note-off time.
                    noteOffTime = note->duration + note->startTime;
                }
                break;
            case MidiEvent::Type::End:
                // for now, should loop.
                assert(false);
                break;
            default:
                assert(false);
        }
        ++curEvent;
    }
}