
#include <assert.h>
#include "ReplaceDataCommand.h"
#include "MidiTrack.h"
#include "MidiSong.h"

ReplaceDataCommand::ReplaceDataCommand(
    std::shared_ptr<MidiSong> song,
    int trackNumber,
    const std::vector<MidiEventPtr>& inRemove,
    const std::vector<MidiEventPtr>& inAdd)
    : song(song), trackNumber(trackNumber), removeData(inRemove), addData(inAdd)
{
    assert(song->getTrack(trackNumber));
}

void ReplaceDataCommand::execute()
{
    MidiTrackPtr mt = song->getTrack(trackNumber);
    assert(mt);

    for (auto it : addData) {
        mt->insertEvent(it);
    }

    for (auto it : removeData) {
        mt->deleteEvent(*it);
    }
}

void ReplaceDataCommand::undo()
{
    MidiTrackPtr mt = song->getTrack(trackNumber);
    assert(mt);

    // to undo the insertion, delete all of them
    for (auto it : addData) {
        mt->deleteEvent(*it);
    }
    for (auto it : removeData) {
        mt->insertEvent(it);
    }
}
