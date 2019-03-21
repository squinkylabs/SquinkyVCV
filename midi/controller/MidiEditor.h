#pragma once

#include "MidiEditorContext.h"
#include <memory>

class MidiEditorContext;
class MidiSelectionModel;
class MidiSequencer;
class MidiSong;
class MidiTrack;

class MidiEditor
{
public:
    MidiEditor(std::shared_ptr<MidiSequencer>);
    MidiEditor(const MidiEditor&) = delete;
    ~MidiEditor();

    /************** functions that move the cursor position ***********/

    void selectNextNote();
    void extendSelectionToNextNote();
    void selectPrevNote();
    void extendSelectionToPrevNote();

    void selectAll();

    /**
     * If ticks is false, will move by "units" (like 1/16 note)
     * amount is a multiplier, and may be negative
     */
    void advanceCursor(bool ticks, int amount);
    void advanceCursorToTime(float time);
    void changeCursorPitch(int semitones);


    /*********** functions that edit/change the notes **************/
    
    void changePitch(int semitones);
    void changeStartTime(bool ticks, int amount);
    void changeDuration(bool ticks, int amount);

    /************* functions that add or remove notes ************/

    enum class Durations {Whole, Half, Quarter, Eighth, Sixteenth };

    void insertPresetNote(Durations);
    void insertNote();
    void deleteNote();

    /*************                                   ***************/
    // Editing start time / duration / pitch
    void setNoteEditorAttribute(MidiEditorContext::NoteAttribute);

    //************** cut / copy / paste ***************/
    void cut();
    void copy();
    void paste();

    void assertCursorInSelection();
     // select any note that is under the cursor
    void updateSelectionForCursor();
private:
    /**
     * The sequencer we will act on.
     * use wek ptr to break ref circle
     */
    std::weak_ptr<MidiSequencer> m_seq;
    std::shared_ptr< MidiSequencer> seq()
    {
        // This assumes of course that m_seq still exists
        auto ret = m_seq.lock();
        assert(ret);
        return ret;
    }

    std::shared_ptr<MidiTrack> getTrack();

    // move the cursor, if necessary.
    void updateCursor();

   

    void extendTrackToMinDuration(float time);

    void insertNoteHelper(Durations dur, bool moveCursorAfter);
};

using MidiEditorPtr = std::shared_ptr<MidiEditor>;

