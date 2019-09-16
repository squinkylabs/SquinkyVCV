#include "Actions.h"

#include "MidiSequencer.h"
#include <assert.h>

Actions::Actions()
{
   _map = {
      {"help", help},
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

      {"select.next", selectNext},
      {"select.next.extend", selectNextExtend},
      {"select.previous", selectPrevious},
      {"select.previous.extend", selectPreviousExtend},

      {"value.increment.normal", valueIncrementNormal}
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

void Actions::handleNoteEditorChange(MidiSequencerPtr sequencer, ChangeType type, bool increase)
{
   int units = 1;
   bool ticks = false;

   // make >,< always advance cursor by ticks i note not selected
   const bool noteSelected = bool(sequencer->editor->getNoteUnderCursor());
   if ((type == ChangeType::small) && !noteSelected) {
      sequencer->editor->advanceCursor(MidiEditor::Advance::Tick, increase ? 1 : -1);
      return;
   }
   
   switch(sequencer->context->noteAttribute) {
      case MidiEditorContext::NoteAttribute::Pitch:
         {
               int semitones = (type == ChangeType::large) ? 12 : 1;
               if (!increase) {
                  semitones = -semitones;
               }
               sequencer->editor->changePitch(semitones);
         }
         break;

      case MidiEditorContext::NoteAttribute::Duration:
         {
               switch(type) {
                  case ChangeType::large:
                     units = 4;
                     ticks = false;
                     break;
                  case ChangeType::small:
                     units = 1;
                     ticks = true;
                     break;
                  case ChangeType::normal:
                     units = 1;
                     ticks = false;
                     break;
                  default:
                     assert(false);
               }
               if (!increase) {
                  units = -units;
               }
               sequencer->editor->changeDuration(ticks, units);
         }
         break;

      case MidiEditorContext::NoteAttribute::StartTime:
         {
               switch(type) {
                  case ChangeType::large:
                     units = 4;
                     ticks = false;
                     break;
                  case ChangeType::small:
                     units = 1;
                     ticks = true;
                     break;
                  case ChangeType::normal:
                     units = 1;
                     ticks = false;
                     break;
                  default:
                     assert(false);

               }
               if (!increase) {
                  units = -units;
               }
               sequencer->editor->changeStartTime(ticks, units);
         }
         break;
   }

}



extern void sequencerHelp();

void Actions::help(MidiSequencerPtr sequencer)
{
   sequencerHelp();
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

//********************* select next note ************


void Actions::selectPrevious(MidiSequencerPtr sequencer)
{
    sequencer->editor->selectNextNote();
}
void Actions::selectPreviousExtend(MidiSequencerPtr sequencer)
{
   sequencer->editor->extendSelectionToPrevNote();
}
void Actions::selectNext(MidiSequencerPtr sequencer)
{
   sequencer->editor->selectNextNote();
}
void Actions::selectNextExtend(MidiSequencerPtr sequencer)
{
   sequencer->editor->extendSelectionToNextNote();
}

//****************** edit note values
void Actions::valueIncrementNormal(MidiSequencerPtr sequencer)
{
   handleNoteEditorChange(sequencer, ChangeType::normal, true);
}