#include "Actions.h"

#include "MidiSequencer.h"
#include <assert.h>

 Actions::Actions()
 {
     _map = {
        {"insert.default", insertDefault},

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

 void Actions::insertDefault(MidiSequencerPtr sequencer)
 {
    sequencer->editor->insertDefaultNote(true, false);
 }

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