# Seq++ Polyphonic piano-roll sequencer

![Seq++ Panel](./seq.png)

## Table of contents

[Major features](#Major-Features)

[About this Sequence](#About-this-Sequencer)

[Grids](#Grids)

[Editing](#Editing)

[Settings](#Settings)

## Major features

Polyphonic sequence of unlimited length.
Graphic “piano roll” note editor. Undo and redo of every change.
Notes may be entered and edited with the mouse or the keyboard.
Notes may be edited while the Sequencer is playing.
Easy to enter and play back “conventional” music.

## About this Sequencer

This sequencer is a hybrid of a modern DAW's MIDI editor, and the MS-DOS sequencer "Sequencer Plus Gold" originally released in 1984. Sequencer Plus was the first popular sequencer to feature a "piano roll" note editor.

So, while Seq++ is much like a typical DAW's MIDI edit screen, it is not exactly like those. You may enter notes with a mouse, as you would expect, but there is also an extensive keyboard interface. Because of this hybrid identity, there are two cursors: the mouse cursor and the blinking keyboard cursor.

The note editor is heavily focused around the keyboard interface, which allows the user to "type in" music very quickly. More conventional note-entry is available with the mouse.

There are some concessions to the current world - it has unlimited undo/redo, and clipboard support.

## Editing
 (see separate doc)

## Grids

The context menu available by right-clicking in the note grid contains several options for the timing grid. 

The grid has two major effects: (1) grid lines are drawn in between the measure lines to help visually align timing;(2) and many edit operations are affected by the grid setting, in conjunction with the snap to grid option.

When snap to grid is enabled, [almost] all cursor positioning is quantized to the nearest grid line.

Note that as the grid units get smaller, not all grid lines will be drawn. For example, at the moment only every other grid line is drawn if the grid is set to sixteenth notes.

Snap duration to grid will ensure that dragging note durations with the mouse will snap the durations. But it will not affect the durations of notes inserted with the keyboard commands (like h, q, and e), since these are already fixed by  the articulation setting. It only affects dragging with the mouse.

## Settings

Right-clicking in the note area will bring up the settings menu. This menu consists of settings (like grid size), as well as some commands, like insert/delete note. All of the settings are at the top, and the commands follow after a blank space.

Settings:

* Grid settings lets you choose the timing units of the edit grid.
* *Snap to grid*. When enabled, the edit cursor is usually snapped to the grid, ensuring that notes will be inserted on the grid unless special effort is employed.
* *Snap duration to grid*. When enabled, dragging note lengths with the mouse will drag onto the nearest grid line.
* *Audition*. When a note is selected, and the sequencer is not running, the note will be played thought channel 1 so that you can hear what you are editing. Audition is on by default, but may be turned off in the settings menu.
* *Articulation*. Determines the duration when a note is in inserted. With 100% articulation, a quarter note would take up the entire duration of a quarter note.

Note that all of the settings are saved with your patch.

Commands:
Insert/Delete note. This menu item changes depending on what is under the cursor. If nothing is there, it will insert a note. If a note is there, it will delete it. (note that double click does that same thing).
Set end point. Will set the end/loop point of the sequencer to the grid line nearest the cursor.

Adjusting length of sequence
Seq++ always plays your sequence in a loop, and the looping comes back around when the “play head” hits the end of the sequence. 

The sequence end is indicated by a vertical purple line in the note grid.

Sequence end point is initialized to the end of the second bar, and can be moved in several ways:
The ‘e’ key will set the end point to the nearest grid point to the edit cursor.
Any edit operation can extend the sequence length to include the results of the edit. So if you drag a note to an area past the end, the end will be extended.
The “set end point” command in the setting menu will do that, too.

For example, if the sequence end is at the end of the second bar, and you move the edit cursor up into bar 10 and insert a note, the edit point will be moved to after the note you just inserted. Similarly, changing the start time or duration of a group of notes may extend the sequence length.
Auditioning notes
By default, every time you select a new note (with cursor or keyboard), it plays on the first output channel. This lets you hear the notes as you are editing. If you don’t like this, it may be turned off in the context menu.

Holding down the tab key will let you quickly hear all the notes in your sequence, one at a time.
Looping, and editing while playing
The looping feature lets you play a range of bars, which can be quite useful when you are editing notes and don’t want to play from the start every time. Looping is toggled on and off with the L key. The loop range is always the range shown in the note editor (the two bar range on the screen).

Looping may be turned on and off while Seq++ is running, and of course you may edit the notes as the sequencer plays. You may also move the loop points in time by moving the edit cursor. Moving by whole bars with the home and end keys is probably the most useful.

It is almost impossible to edit while playing if the playback scrolling is turned on. When scrolling the sequencer will move the edit cursor all the time, making it very difficult for you to edit.

While Seq++ is playing you can turn loop on and off, freely move the time range, and use all the edit commands. It will not glitch or get out of sync. Switching looping off can be jarring, however, as the play point will immediately go to where it would have been if you never looped - often far past the current loop range.

Keyboard focus
As mentioned before, the UI is heavily keyboard driven. The module will only respond to the keyboard if the mouse cursor is over the note editor, in which case it will grab the focus as you type. Once the module has focus it keeps it until you click outside. Normally this means that the mouse can move anywhere outside the module, but the keyboard will still control Seq++.

But there are some keys that VCV Rack itself tries very hard to steal from Seq++, like the cursor keys and the backspace key. For these keys, the mouse must stay over the piano roll for Seq++ to get those keys. 

That is why for all these keys there are substitute keys that also work, and do not have to fight with VCV for control of the keyboard. Both 4 keys will cursor to the left, and both 6 keys will cursor to the right.
Panel 
On the left are a few inputs, outputs, and controls.

**Clock Rate** dropdown determines the clocking. These settings tell the sequencer how to interpret the external clock input.

**Polyphony** dropdown selects how many voices will be played back.

**Run/Stop** button that changes color to  indicate when it's running. The run state is controlled from this button, as well as the external run input. Either of them can start and stop the sequencer. This button is modeled on the run button in "Clocked", from Impromptu Modular.

**Scroll mode** button enabled scrolling while running.

**CV** is the [polyphonic] pitch CV output. It will be polyphonic if Seq++ is set for more than one voice.

**G** is the gate output. It will be polyphonic if Seq++ is set for more than one voice.

**Clk** is the eternal clock input.

**Rst** is the reset CV. It is quite compatible with the reset out of "Clocked".

**Run** is a CV input. Meant to be hooked up to the Run output of Clocked, or similar master clock.

Clocking and playback
Always keep the following in mind:

* The Clock Rate control only affectcs playback.
* The "grid" is merely a graphic overlay drawn on top of the edit window. It has no effect on playback. Ever.
* If "snap to grid" is enabled, some note editing operations will quantize to the grid, but it will still have no effect at all at playback time.

You will need to plug in a clock source. Almost anything will do - even an LFO. *Clocked* from Impromptu Modular is an excellent choice, so we will describe how to get the best results using Clocked. But many other clock generators will work.

Connect the clock, run and reset outputs of Clocked to the corresponding inputs of Seq++.

Set Seq++'s clock rate to match the *Ratio* in clocked. So if Clocked is set for X4, set Seq++ to *X4 1/16".

For DAW workflows, or other times when you want fine timing, use as fine a clock resolution as possible (Clocked X64). To intentionally coarsely-quantize Seq++, use a less fine clock.

When Seq++ quantizes during playback, it should quantize "correctly" - i.e., it should quantize to the **nearest** clock. Both the start time and duration will be quantized.

If two notes of the same pitch "touch" each other, such that the end of one note is at the same time as the start of the next, Seq++ will re-trigger the note by bringing the gate low for a millisecond. Note that with quantization it is very common for two notes to touch this way.

If the sequence is set for monophonic, then it should play a proper legato if the notes fully overlap (the second one starts after the first one ends). But remember that quantization may erase this overlap, causing the re-trigger to kick in.

If there are more simultaneous notes being played than may be accommodated by the polyphony setting, then voices will get "stolen" and re-assigned to play the new notes. At the moment Seq++ uses a variation on "last note" assignment. If there is already an idle voice that was playing the new pitch, that voice will be selected. Then the next idle voice (in rotation) will be used, then the first voice will be used.

Keyboard reference
Mouse reference


