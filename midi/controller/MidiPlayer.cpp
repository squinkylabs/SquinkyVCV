
#include "MidiPlayer.h"

void MidiPlayer::timeElapsed(float seconds)
{
    curMetricTime += seconds * 120.0f / 60.0f;        // fixed at 120 bpm for now

    while (playOnce()) {
    }
}

bool MidiPlayer::playOnce()
{
    if (!isPlaying) {
        return false;
    }
    bool didSomething = false;
    if (noteOffTime >= 0 && noteOffTime <= curMetricTime) {
        host->setGate(false);
        noteOffTime = -1;
        didSomething = true;
    }

    if (curEvent->first <= curMetricTime) {
        MidiEventPtr event = curEvent->second;
        switch (event->type) {
            case MidiEvent::Type::Note:
            {
                MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(event);
                // should now output the note.
                host->setGate(true);
                host->setCV(note->pitchCV);

                // and save off the note-off time.
                noteOffTime = note->duration + note->startTime;
                ++curEvent;
            }
            break;
            case MidiEvent::Type::End:
                // for now, should loop.
                curMetricTime = 0;
                curEvent = song->getTrack(0)->begin();
                break;
            default:
                assert(false);
        }

        didSomething = true;
    }
    return didSomething;

}