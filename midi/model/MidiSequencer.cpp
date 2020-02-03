
#include "ISeqSettings.h"
#include "MidiSequencer.h"
#include "MidiEditor.h"
#include "TimeUtils.h"
#include "UndoRedoStack.h"

int _mdb = 0;       // global instance counter

MidiSequencer::MidiSequencer(MidiSongPtr sng, ISeqSettingsPtr setp, IMidiPlayerAuditionHostPtr auditionHost) :
    selection(std::make_shared<MidiSelectionModel>(auditionHost)),
    song(sng),
    context(std::make_shared<MidiEditorContext>(sng, setp))
{
    // init the context to something reasonable.
    context->setEndTime(TimeUtils::bar2time(2));
    undo = std::make_shared<UndoRedoStack>();
    ++_mdb;
}

 void MidiSequencer::setNewSong(MidiSongPtr song)
 {
     this->song = song;

     // I think only context needs to be updated.
     this->context->setNewSong(song);
 }

MidiSequencerPtr MidiSequencer::make(MidiSongPtr song, std::shared_ptr<ISeqSettings> settings, IMidiPlayerAuditionHostPtr audition)
{
    assert(settings);
    MidiSequencerPtr seq(new MidiSequencer(song, settings, audition));
    seq->makeEditor();

    // Find a track to point the edit context at
    bool found = false;
    int maxTk = song->getHighestTrackNumber();
    for (int i = 0; i <= maxTk; ++i) {
        if (song->trackExists(i)) {
            seq->context->setTrackNumber(i);
            found = true;
            break;
        }
    }
    (void) found;
    assert(found);
    seq->context->setPitchLow(0);
    seq->context->setPitchHi(2);

    seq->assertValid();
    return seq;
}
 

void MidiSequencer::makeEditor()
{
    MidiSequencerPtr seq = shared_from_this();
    editor = std::make_shared<MidiEditor>(seq);
}

MidiSequencer::~MidiSequencer()
{
    --_mdb;
}


void MidiSequencer::assertValid() const
{
#ifndef NDEBUG
    assert(editor);
    assert(undo);
    assert(song);
    assert(context);
    assert(selection);
    song->assertValid();
    context->assertValid();
    assertSelectionInTrack();

    const int trackNumber = context->getTrackNumber();
    MidiTrack* track = this->context->getTrack().get();
    MidiTrack* track2 = this->song->getTrack(trackNumber).get();
    (void) track;
    (void) track2;
    
    assert(track == track2);
#endif
}

void MidiSequencer::assertSelectionInTrack() const
{
    MidiTrackPtr track = context->getTrack();
    
    for (auto it : *selection) {
        auto foundPtr = track->findEventPointer(it);
        assert(foundPtr != track->end());
        auto x = *foundPtr;
        MidiEventPtrC y = x.second;
    }
}

