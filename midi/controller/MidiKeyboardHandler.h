#pragma once

#include "Seq.h"
#include "WidgetComposite.h"
#include <memory>

class MidiSequencer;
class WidgetComposite;
using MidiSequencerPtr = std::shared_ptr<MidiSequencer>;

class MidiKeyboardHandler
{
public:
    /**
     * returns true if this key implements repeat
     */
    static bool doRepeat(unsigned key);
    static bool handle(MidiSequencerPtr sequencer, unsigned key, unsigned mods);

    /**
     * Let's put the mouse handlers in here, too
     */
    static void doMouseClick( MidiSequencerPtr sequencer, float time, float pitchCV,
        bool shiftKey, bool ctrlKey);

    static void onUIThread(std::shared_ptr<Seq<WidgetComposite>> seqComp);
private:
    enum class ChangeType { lessThan, plus, bracket };
   
    static void handleNoteEditorChange(MidiSequencerPtr sequencer, ChangeType type, bool increase);

    class StepRecordImp
    {

    };

    static StepRecordImp stepRecordImp;
};
