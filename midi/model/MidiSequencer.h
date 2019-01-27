#pragma once

#include "MidiEditor.h"
#include "MidiSelectionModel.h"
#include "MidiSong.h"
#include "MidiEditorContext.h"
#include <memory>

class MidiSong;

/**
 * Despite the fancy name, this class doesn't do much.
 * It holds all the other modules that make up a sequencer.
 * It knows how to create all the objects and hook them up.
 */
class MidiSequencer
{
public:
    /** constructor takes a song to edit
    */
    MidiSequencer(std::shared_ptr<MidiSong>);
    ~MidiSequencer();

    void assertValid() const;

    /** The various classes we collect
     */
    MidiSelectionModelPtr const selection;
    MidiSongPtr const song;
    MidiEditorContextPtr context;
    MidiEditorPtr const editor;
};

using MidiSequencerPtr = std::shared_ptr<MidiSequencer>;