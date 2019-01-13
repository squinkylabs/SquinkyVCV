#pragma once
#include <memory>

class MidiSelectionModel;
class MidiSong;
class MidiTrack;

class MidiEditor
{
public:
    MidiEditor(std::shared_ptr<MidiSong>, std::shared_ptr<MidiSelectionModel> selection);
    ~ MidiEditor();
    void selectNextNote();
private:
    /**
     * The selection model we will act on
     */
    std::shared_ptr<MidiSelectionModel> selection;

    std::shared_ptr<MidiSong> song;

    std::shared_ptr<MidiTrack> getTrack();


};

using MidiEditorPtr = std::shared_ptr<MidiEditor>;

