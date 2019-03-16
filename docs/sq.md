# Seq
Work in progress

Keyboard mappings [here](./keymap.md)

## Keyboard focus

The UI is entirely keyboard driven, no mouse. And the module will not receive keyboard input until you click in the note-grid. Then it will steal keyboard focus until you click somewhere else.

## The panel side

On the left are a few inputs, outputs, and controls.

**Clock Rate** dropdown determines the clocking. Most of the settings tell the sequencer how to interpret the external clock input. Of course when it's set to internal it's a different story.

**Tempo** determines the tempo when the Clock Rate is set to internal.

**Run/Stop** button. At the moment it is labeled "preset". It has a green LED next to it that indicates running. This is modeled on the run button in clocked.

**Scroll mode** button enabled scrolling while running.

**CV** is the pitch CV output.

**G** is the gate output. When it is active the gree LED lights.

**Clk** is the eternal clock input.

**Rst** does nothing yet.

**Run** CV input. Meant to be hooked up to the Run output of clocked. I haven't tested it yet.

## What works now

Plays sequence looped.

Edit note attributes (pitch, start time, duration)

Draw Piano roll.

Undo / Redo. Every operation that changes the note data may be undone.

Copy/Paste (you can even copy from one instance and paste to another);

There is a single track, and it is monophonic.

## What doesn't work

There are obviously many missing features (start/stop, advanced editing). There is no mouse interface. It looks pretty rough.

Note editor things that don't work:

* Time units are always 1/16 notes.
* Insert note is always 1/4 (I think).

Reset input not hooked up.

It is probably possible to crash it with some note editor operations. But please report such things to me.

## Extending the length

There is a temporary hack to make it possible to lengthen a track. You may move the cursor past the end of the track. If you insert a note there, the track will be extended in units of 4/4 bars to accommodate the new note.

## Piano roll

It works like you would expect. Details are in [Keyboard Summary](./keymap.md).

Note that there is a blinking "DOS cursor". It is not connected to the mouse cursor in any way. All note editing is done via the keyboard and this cursor.

## Features I plan to add before release

Address all the issues mentioned above.

Add a minimal mouse interface to the note editor.

Integrate undo/redo with the undo/redo in VCV 1.0.

Do something about the keyboard focus so you don't always need to click in the grid.
