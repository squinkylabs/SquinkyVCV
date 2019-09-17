#include "Actions.h"
#include "ActionContext.h"

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

 void Actions::handleInsertPresetNote(
    ActionContext& context,
    MidiEditor::Durations duration, 
    bool advanceAfter)
{
   MidiSequencerPtr sequencer = context.sequencer;
   assert(sequencer);

   // First, see if step record wants this event.
   bool handled = context.handleInsertPresetNote(sequencer, duration, advanceAfter);
   if (!handled) {
      sequencer->editor->insertPresetNote(duration, advanceAfter);
   }
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

void Actions::help(ActionContext& context)
{
   sequencerHelp();
}

void Actions::insertDefault(ActionContext& context)
{
   context.sequencer->editor->insertDefaultNote(true, false);
}

void Actions::insertHalfAdvance(ActionContext& context)
{
   handleInsertPresetNote(context, MidiEditor::Durations::Half, true);
}

void Actions::insertWholeAdvance(ActionContext& context)
{
   handleInsertPresetNote(context, MidiEditor::Durations::Whole, true);
}

void Actions::insertQuarterAdvance(ActionContext& context)
{
   handleInsertPresetNote(context, MidiEditor::Durations::Quarter, true);
}

void Actions::insertEighthAdvance(ActionContext& context)
{
   handleInsertPresetNote(context, MidiEditor::Durations::Eighth, true);
}

void Actions::insertSixteenthAdvance(ActionContext& context)
{
   handleInsertPresetNote(context, MidiEditor::Durations::Sixteenth, true);
}

void Actions::insertHalf(ActionContext& context)
{
   handleInsertPresetNote(context, MidiEditor::Durations::Half, false);
}

void Actions::insertWhole(ActionContext& context)
{
handleInsertPresetNote(context, MidiEditor::Durations::Whole, false);
}

void Actions::insertQuarter(ActionContext& context)
{
   handleInsertPresetNote(context, MidiEditor::Durations::Quarter, false);
}

void Actions::insertEighth(ActionContext& context)
{
   handleInsertPresetNote(context, MidiEditor::Durations::Eighth, false);
}

void Actions::insertSixteenth(ActionContext& context)
{
   handleInsertPresetNote(context, MidiEditor::Durations::Sixteenth, false);
}
//******************** cursor movement ****************************

void Actions::moveLeftNormal(ActionContext& context)
{
   context.sequencer->editor->advanceCursor(MidiEditor::Advance::GridUnit, -1);  
}

void Actions::moveRightNormal(ActionContext& context)
{
   context.sequencer->editor->advanceCursor(MidiEditor::Advance::GridUnit, 1);  
}


void Actions::moveUpNormal(ActionContext& context)
{
   context.sequencer->editor->changeCursorPitch(1);
}

void Actions::moveDownNormal(ActionContext& context)
{
   context.sequencer->editor->changeCursorPitch(-1);
}

//********************* select next note ************


void Actions::selectPrevious(ActionContext& context)
{
    context.sequencer->editor->selectNextNote();
}
void Actions::selectPreviousExtend(ActionContext& context)
{
   context.sequencer->editor->extendSelectionToPrevNote();
}
void Actions::selectNext(ActionContext& context)
{
   context.sequencer->editor->selectNextNote();
}
void Actions::selectNextExtend(ActionContext& context)
{
   context.sequencer->editor->extendSelectionToNextNote();
}

//****************** edit note values
void Actions::valueIncrementNormal(ActionContext& context)
{
   handleNoteEditorChange(context.sequencer, ChangeType::normal, true);
}