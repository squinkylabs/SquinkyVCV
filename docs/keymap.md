# Sequencer keyboard commands

## Moving around

**cursor keypad**: moves cursor in two dimensions. Up and down by semitone, left and right by one time unit.

**ctrl-cursor**: moves left and right by four units.

## Selecting notes

**ctrl-a** selects all the events in the track.

**tab**: select next note.

**ctrl-tab**: select previous note. (used to be shift tab).

**shift-tab**: extends selection to next note.

**ctrl-shift-tab**: extends selection to the previous note.

Moving the cursor onto a note will select it.

## Changing notes

**S, P, D**: sets note attribute to be edited (Start time, Pitch, and Duration).

When notes are selected and StartTime or Duration is current edit attribute:

* plus/minus changes by one sixteenth note

* ], [ changes by a quarter note.

* <, > change by a sixty-fourth note.

When note is selected and Pitch is current edit attribute:

* plus/minus changes by one semitone.
* ], [ changes by an octave.

## Inserting and deleting notes

**Ins** inserts a note at the current cursor.

**Del** deletes the currently selected notes.

Insert preset note durations. They shortcuts insert note of a specific duration, then move the cursor past that location.

* **ctrl-w** Whole note.
* **ctrl-h** Half note.
* **ctrl-q** Quarter note.
* **ctrl-e** Eighth note.
* **ctrl-s** Sixteenth note.

## Cut/Copy/Paste

**ctrl-x** cut. Removes all the selected notes and puts them on the clipboard.

**ctrl-c** copy. Puts a copy of all the selected notes on the clipboard.

**ctrl-v** paste. Pastes the contents of the clipboard at the current edit cursor.

Note that you may paste into a different instance of the sequencer than you copied from, as you would expect.

## Undo/Redo

**ctrl-z**: undo

**ctrl-y, shift-ctrl-z**: redo

## Help

**F1 key**, when note editor has focus.

**Context menu**, when the module has focus.
