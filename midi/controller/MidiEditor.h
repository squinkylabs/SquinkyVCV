#pragma once

#include "MidiEditorContext.h"
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
    ~MidiEditor();

    /************** functions that move the cursor position ***********/

    void selectNextNote();
    void selectPrevNote();
    /**
     * If ticks is false, will move by "units" (like 1/16 note)
     * amount is a multiplier, and may be negative
     */
    void advanceCursor(bool ticks, int amount);


    /*********** functions that edit/change the notes **************/
    void changeCursorPitch(int semitones);
    void changePitch(int semitones);
    void changeStartTime(bool ticks, int amount);
    void changeDuration(bool ticks, int amount);
    void insertNote();
    void deleteNote();

    /*************                                   ***************/
    // Editing start time / duration / pitch
    void setNoteEditorAttribute(MidiEditorContext::NoteAttribute);

    void assertCursorInSelection();
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

    // select any note that is under the cursor
    void updateSelectionForCursor();

    void extendTrackToMinDuration(float time);
};

using MidiEditorPtr = std::shared_ptr<MidiEditor>;

