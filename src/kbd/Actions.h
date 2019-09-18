#pragma once

#include "MidiEditor.h"
#include <functional>
#include <map>
#include <memory>
#include <string>

class ActionContext;
class MidiSequencer;
using MidiSequencerPtr = std::shared_ptr<MidiSequencer>;

class Actions
{
public:
    Actions();
    using action = std::function<void(ActionContext&)>;
    action getAction(const std::string& name);
private:

    std::map< std::string, action> _map;

    enum class ChangeType { small, normal, large };
    static void handleNoteEditorChange(MidiSequencerPtr sequencer, ChangeType type, bool increase);
    static void handleInsertPresetNote(
        ActionContext& context,
        MidiEditor::Durations duration, 
        bool advanceAfter);
    /**
     * all of the actions
     */
    static void insertDefault(ActionContext&);
    static void insertWhole(ActionContext&);
    static void insertHalf(ActionContext&);
    static void insertQuarter(ActionContext&);
    static void insertEighth(ActionContext&);
    static void insertSixteenth(ActionContext&);
    static void insertWholeAdvance(ActionContext&);
    static void insertHalfAdvance(ActionContext&);
    static void insertQuarterAdvance(ActionContext&);
    static void insertEighthAdvance(ActionContext&);
    static void insertSixteenthAdvance(ActionContext&);
    static void moveLeftNormal(ActionContext&);
    static void moveUpNormal(ActionContext&);
    static void moveDownNormal(ActionContext&);
    static void moveRightNormal(ActionContext&);

    static void selectPrevious(ActionContext&);
    static void selectPreviousExtend(ActionContext&);
    static void selectNext(ActionContext&);
    static void selectNextExtend(ActionContext&);
    static void help(ActionContext&);

    static void valueIncrementSmall(ActionContext&);
    static void valueIncrementNormal(ActionContext&);
    static void valueIncrementLarge(ActionContext&);

    static void valueDecrementSmall(ActionContext&);
    static void valueDecrementNormal(ActionContext&);
    static void valueDecrementLarge(ActionContext&);

    static void selectAll(ActionContext&);

    static void cut(ActionContext&);
    static void copy(ActionContext&);
    static void paste(ActionContext&);

    static void editDuration(ActionContext&);
    static void editPitch(ActionContext&);
    static void editStartTime(ActionContext&);

    static void loop(ActionContext&);
    static void changeTrackLength(ActionContext&);

    static void grabDefaultNote(ActionContext&);
    static void deleteNote(ActionContext&);
};