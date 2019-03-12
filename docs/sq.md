# S 
Work in progress

Keyboard mappings [here](./keymap.md)

## Keyboard focus

The UI is entirely keyboard driven, no mouse. And the module will not receive keyboard input until you click in the note-grid. Then it will steal keyboard focus until you click somewhere else.

## The panel side

On the left are a few inputs, outputs, and controls.

**Clock Rate** dropdown determines the clocking. Most of the settings tell the sequencer how to interpret the external clock input. Of course when it's set to internal it's a different story.

**Tempo** determines the tempo when the Clock Rate is set to internal.

**Run/Stop** button does nothing yet.

**Scroll mode** button does nothing yet.

**CV** is the pitch CV output.

**G** is the gate output. When it is active the gree LED lights.

**Clk** is the eternal clock input.

**Rst** does nothing yet.

**Run** does nothing yet.

## What works now

Plays sequence looped all the time.

Edit note attributes (pitch, start time, duration)

Draw Piano roll.

Undo / Redo.

Cut/Copy/Paste

There is a single track, and it is monophonic.

## What doesn't work.

There are obviously many missing features (start/stop, advanced editing). There is no mouse interface. It looks pretty rough.

Note editor things that don't work:

* Time units are always 1/16 notes.
* Insert note is always 1/4 (I think).

## Extending the length

There is a temporary hack to make it possible to lengthen a track. You may move the cursor past the end of the track. If you insert a note there, the track will be extended in units of 4/4 bars to accommodate the new note.

## Piano roll

It works like you would expect. Details are in [Keyboard Summary](./keymap.md).

Note that there is a blinking "DOS cursor". It is not connected to the mouse cursor in any way. All note editing is done via the keyboard and this cursor.