# Sequencer keyboard commands

Please note that everything here has changed August 1 2019.

Many key commands take input from the grid size set from the settings menu.

## Misc

**n**: sets the end point of the sequence to the current cursor time. Time is always quantized to the grid, even if snap to grid is off.

**l**: Loops a range of bars from the track. Loop range is the bars that are on screen. May be turned on and off while playing. Moving the "viewport" to a different range of bars will change the loop range, even if you are playing when you do it.

## Moving around

**cursor keypad**: moves cursor in two dimensions. Up and down by semitone, left and right by one grid unit.

**ctrl-cursor**: moves left and right by a quarter note.

**home, end**: moved on bar earlier or later.

**ctrl-hom, end**: moves to first bar or last bar in the sequence.

**PgUp, PgDn**: moves up or down by an octave.

## Selecting notes

**ctrl-a** selects all the events in the track.

**tab**: select next note.

**ctrl-tab**: select previous note. (used to be shift tab).

**shift-tab**: extends selection to next note.

**ctrl-shift-tab**: extends selection to the previous note.

Moving the cursor onto a note will select it.

## Inserting and deleting notes

Note that are inserted will have their start time quantized to the grid if snap to grid is enabled in the settings menu. After note is inserted the cursor will be advanced past the note just inserted, unless the shift key is held down.

**Ins or Enter** inserts a note at the current cursor. Duration will be one grid unit.

**Del** deletes the currently selected notes.

Insert preset note durations. They shortcuts insert note of a specific duration.

* **w** Whole note.
* **h** Half note.
* **q** Quarter note.
* **e** Eighth note.
* **x** Sixteenth note. Note that 's' key is already used for Start time, so 'x' is used for sixteenth note. Ctrl-s will also work.

## Changing notes

**S, P, D**: sets note attribute to be edited (Start time, Pitch, and Duration).

When notes are selected and StartTime or Duration is current edit attribute:

* plus/minus changes by one sixteenth note

* ], [ changes by a quarter note.

* <, > change by a sixty-fourth note.

When note is selected and Pitch is current edit attribute:

* plus/minus changes by one semitone.
* ], [ changes by an octave.

## Cut/Copy/Paste

**ctrl-x** cut. Removes all the selected notes and puts them on the clipboard. (doesn't work yet).

**ctrl-c** copy. Puts a copy of all the selected notes on the clipboard.

**ctrl-v** paste. Pastes the contents of the clipboard at the current edit cursor.

Note that you may paste into a different instance of the sequencer than you copied from, as you would expect.

## Undo/Redo

VCV 1.0, Seq++ uses VCV's undo system, which is available from a mouse menu and from keyboard shortcuts.

**ctrl-z**: undo

**ctrl-y, shift-ctrl-z**: redo

## Help

**F1 key**, when note editor has focus.

**Context menu**, when the module has focus.
