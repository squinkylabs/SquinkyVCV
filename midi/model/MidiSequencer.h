#pragma once

#include "MidiEditor.h"
#include "MidiSelectionModel.h"
#include "MidiSong.h"
#include "MidiEditorContext.h"
#include "UndoRedoStack.h"
#include <memory>

class ISeqSettings;
class MidiSong;
class MidiSequencer;
class IMidiPlayerAuditionHost;
using MidiSequencerPtr = std::shared_ptr<MidiSequencer>;
using IMidiPlayerAuditionHostPtr = std::shared_ptr<IMidiPlayerAuditionHost>;

/**
 * Despite the fancy name, this class doesn't do much.
 * It holds all the other modules that make up a sequencer.
 * It knows how to create all the objects and hook them up.
 */
class MidiSequencer : public std::enable_shared_from_this<MidiSequencer>
{
public:
    
    static MidiSequencerPtr make(
        std::shared_ptr<MidiSong>,
        std::shared_ptr<ISeqSettings>,
        IMidiPlayerAuditionHostPtr audition);
    ~MidiSequencer();

    void assertValid() const;

    /** The various classes we collect
     */
    MidiSelectionModelPtr const selection;

    /**
     * This used to be a const ptr, but to load 
     * midi file we need to set it. But external
     * code should not do this directly - should use setNewSong()
     */
    MidiSongPtr song;
    MidiEditorContextPtr context;
    MidiEditorPtr editor;
    UndoRedoStackPtr undo;

    /**
     * Accepts a new song, notifying various
     * children of the new song.
     */
    void setNewSong(MidiSongPtr song);
protected:

private:
    void assertSelectionInTrack() const;

    /** constructor takes a song to edit
    */
    MidiSequencer(std::shared_ptr<MidiSong>, std::shared_ptr<ISeqSettings>, IMidiPlayerAuditionHostPtr);
    MidiSequencer() = delete;
    MidiSequencer(const MidiSequencer&) = delete;

    /**
     * must be called to make constructor
     * todo: memory leak for circular ref
     */
    void makeEditor();

};

