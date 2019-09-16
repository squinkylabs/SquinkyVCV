#include "Actions.h"

#include "MidiSequencer.h"
#include <assert.h>

Actions::Actions()
{
   _map = {
      {"insert.default", insertDefault},
      {"insert.whole.advance", insertWholeAdvance},
      {"insert.half.advance", insertHalfAdvance},
      {"insert.quarter.advance", insertQuarterAdvance},
      {"insert.eighth.advance", insertEighthAdvance},
      {"insert.sixteenth.advance", insertSixteenthAdvance},

      {"insert.whole", insertWhole},
      {"insert.half", insertHalf},
      {"insert.quarter", insertQuarter},
      {"insert.eighth", insertEighth},
      {"insert.sixteenth", insertSixteenth},


      {"move.left.normal", moveLeftNormal},
      {"move.right.normal", moveRightNormal},
      {"move.up.normal", moveUpNormal},
      {"move.down.normal", moveDownNormal},
   };
}

Actions::action Actions::getAction(const std::string& name)
{
   auto it = _map.find(name);
   if (it == _map.end()) {
      fprintf(stderr, "bad action name: %s\n", name.c_str());
      return nullptr;
   }
   
   action a = it->second;
   return a;
}


/********************** Note insert ******************************/

 static void handleInsertPresetNote(
    MidiSequencerPtr sequencer,
    MidiEditor::Durations duration, 
    bool advanceAfter)
{
    // First, see if step record wants this event.
   // bool handled = stepRecordImp.handleInsertPresetNote(sequencer, duration, advanceAfter);
   // if (!handled) {
     sequencer->editor->insertPresetNote(duration, advanceAfter);
   // }
}

void Actions::insertDefault(MidiSequencerPtr sequencer)
{
   sequencer->editor->insertDefaultNote(true, false);
}

void Actions::insertHalfAdvance(MidiSequencerPtr sequencer)
{
   handleInsertPresetNote(sequencer, MidiEditor::Durations::Half, true);
}

void Actions::insertWholeAdvance(MidiSequencerPtr sequencer)
{
handleInsertPresetNote(sequencer, MidiEditor::Durations::Whole, true);
}

void Actions::insertQuarterAdvance(MidiSequencerPtr sequencer)
{
   handleInsertPresetNote(sequencer, MidiEditor::Durations::Quarter, true);
}

void Actions::insertEighthAdvance(MidiSequencerPtr sequencer)
{
   handleInsertPresetNote(sequencer, MidiEditor::Durations::Eighth, true);
}

void Actions::insertSixteenthAdvance(MidiSequencerPtr sequencer)
{
   handleInsertPresetNote(sequencer, MidiEditor::Durations::Sixteenth, true);
}

void Actions::insertHalf(MidiSequencerPtr sequencer)
{
   handleInsertPresetNote(sequencer, MidiEditor::Durations::Half, false);
}

void Actions::insertWhole(MidiSequencerPtr sequencer)
{
handleInsertPresetNote(sequencer, MidiEditor::Durations::Whole, false);
}

void Actions::insertQuarter(MidiSequencerPtr sequencer)
{
   handleInsertPresetNote(sequencer, MidiEditor::Durations::Quarter, false);
}

void Actions::insertEighth(MidiSequencerPtr sequencer)
{
   handleInsertPresetNote(sequencer, MidiEditor::Durations::Eighth, false);
}

void Actions::insertSixteenth(MidiSequencerPtr sequencer)
{
   handleInsertPresetNote(sequencer, MidiEditor::Durations::Sixteenth, false);
}
//******************** cursor movement ****************************

void Actions::moveLeftNormal(MidiSequencerPtr sequencer)
{
   sequencer->editor->advanceCursor(MidiEditor::Advance::GridUnit, -1);  
}

void Actions::moveRightNormal(MidiSequencerPtr sequencer)
{
   sequencer->editor->advanceCursor(MidiEditor::Advance::GridUnit, 1);  
}


void Actions::moveUpNormal(MidiSequencerPtr sequencer)
{
   sequencer->editor->changeCursorPitch(1);
}

void Actions::moveDownNormal(MidiSequencerPtr sequencer)
{
   sequencer->editor->changeCursorPitch(-1);
}