
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

    for (auto it = addData.begin(); it < addData.end(); ++it) {
        mt->insertEvent(*it);
    }
    assert(removeData.empty()); // not imp yet
}

void ReplaceDataCommand::undo()
{
    MidiTrackPtr mt = song->getTrack(trackNumber);
    assert(mt);

    // to undo the insertion, delete all of them
    for (auto it = addData.begin(); it < addData.end(); ++it) {
        mt->deleteEvent(**it);
    }
    assert(removeData.empty());
}
