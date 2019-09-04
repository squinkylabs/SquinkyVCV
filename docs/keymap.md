# Sequencer keyboard commands

Many key commands take input from the grid size set from the settings menu. Commands that are mapped to letter keys will usally work with either lower or upper case. In some selection operations, however, the shift key is used to extend a selection.

## Misc

**n**: Sets the end point of the sequence to the current cursor time. Time is always quantized to the grid, even if snap to grid is off.

**l**: Loops a range of bars from the track. Loop range is the bars that are on screen. May be turned on and off while playing. Moving the "viewport" to a different range of bars will change the loop range, even if you are playing when you do it.

## Moving around

**cursor keypad**: Moves cursor in two dimensions. Up and down by semitone, left and right by one grid unit.

**ctrl-cursor**: Moves left and right by a quarter note.

**4 and 6** keys also move left and right by one grid unit, or a quarter note if the ctrl key is down. These alternate "cursor" keys are useful, as they will still work if the mouse isn't in the editor. Also since these keys are on both sides of most keyboard, one location will be comfortable for either left or right handed users.

**home, end**: Moved one bar earlier or later.

**ctrl-home, end**: Moves to first bar or last bar in the sequence.

**PgUp, PgDn**: Moves up or down by an octave.

## Selecting notes

**ctrl-a** Selects all the events in the track.

**tab**: Select next note.

**ctrl-tab**: Select previous note.

**shift-tab**: Extends selection to include the next note.

**ctrl-shift-tab**: extends selection to the previous note.

Moving the cursor onto a note will select it.

## Inserting and deleting notes

When a note is inserted, its start time and duration will not be quantized, no matter the snap to gird settings. But since most cursor movement WILL qunatize the cursor time when snap to grid is on, notes will naturally tend to be inserted on the grid. But it is possible to move off the grid and then use all the insert commands.

After a note is inserted the cursor will be advanced past the note just inserted, unless the shift key is held down.

**Ins or Enter** Inserts a note at the current cursor. Duration will be one grid unit by default, but my be set to whatever you want with the asterisk key.

**Del** Deletes the currently selected notes.

Insert preset note durations. They shortcuts insert note of a specific duration.

* **w** Whole note.
* **h** Half note.
* **q** Quarter note.
* **e** Eighth note.
* **x** Sixteenth note. Note that 's' key is already used for Start time, so 'x' is used for sixteenth note. Ctrl-s will also work.

* **asterisk** (*) will take the duration of the note under the cursor and use that as the duration for all subsequent notes inserted without explicit duration (i.e. from the Ins and Enter keys, and from double-clicking the mouse.

**Very Important**: by default the cursor will advance after inserting a note, making it easier to insert a stream on notes in succession. But if you hold down the shift key the cursor will not move ahead, making it easy to insert chords.

## Changing notes

**S, P, D**: sets note attribute to be edited (Start time, Pitch, and Duration). The current mode is always displayed in the status area above the note grid.

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

Seq++ uses VCV's undo system, which is available from a mouse menu and from keyboard shortcuts.

**ctrl-z**: undo

**ctrl-y, shift-ctrl-z**: redo

## Help

**F1 key**, when note editor has focus.

**Context menu**, when the module has focus.
