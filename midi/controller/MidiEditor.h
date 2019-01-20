#pragma once
#include <memory>

class MidiEditorContext;
class MidiSelectionModel;
class MidiSong;
class MidiTrack;

class MidiEditor
{
public:
    MidiEditor(
        std::shared_ptr<MidiSong>,
        std::shared_ptr<MidiSelectionModel> selection,
        std::shared_ptr<MidiEditorContext> context);
    MidiEditor(const MidiEditor&) = delete;
    ~ MidiEditor();
    void selectNextNote();
    void selectPrevNote();
    void transpose(float amount);
private:
    /**
     * The selection model we will act on
     */
    std::shared_ptr<MidiSelectionModel> selection;
    std::shared_ptr<MidiSong> song;
    std::shared_ptr<MidiEditorContext> context;

    std::shared_ptr<MidiTrack> getTrack();

    // move the cursor, if necessary.
    void updateCursor();


};

using MidiEditorPtr = std::shared_ptr<MidiEditor>;

