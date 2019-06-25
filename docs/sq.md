# Seq++

Work in progress

If you read nothing else, please checkout out the list of keyboard "shortcuts". This is the only way you can hope to figure out how to enter notesquickly: Keyboard mappings [here](./keymap.md)

## About this Sequencer

This sequencer is modeled after the MS-DOS sequencer "Sequencer Plug Gold" originally released in 1984. Sequencer Plus was the first popular sequencer to feature a "piano roll" note editor.

So, while Seq++ is much like a typical DAW's MIDI edit screen, it is not exactly like those. The note editor is heavily focused around the keyboard interface, which allows the user to "type in" music very quickly. More conventional note-entry is available with the mouse.

It can deal with tracks that are arbitrarily long and dense. User may edit the notes as the sequencer is playing.

There are some concessions to the current world - it has unlimited undo/redo, and clipboard support.

## More about undo/redo

Seq++ integrates fully with VCV Rack's undo/redo system. Try inserting a note, then open the edit menu. It will say `undo Seq++: insert note`.

## More about the clipboard

If you cut or copy a range of notes, you may insert your cursor in a new location, and paste there. You can even go do a second instance of Seq++ and paste into that.

## Keyboard focus

The UI is entirely keyboard driven, no mouse. The module will only respond the the keyboard if the cursor is over the note editor, in which case it will grab the focus as you type. Once the module had focus it keeps it until you click outside.

Because it's easy to forget, the edit grid had a conspicuous indicator that tells when it has keyboard focus.

## The panel side

On the left are a few inputs, outputs, and controls.

**Clock Rate** dropdown determines the clocking. Most of the settings tell the sequencer how to interpret the external clock input. Of course when it's set to internal it's a different story.

**Tempo** determines the tempo when the Clock Rate is set to internal.

**Run/Stop** button that changes color to  indicate when it's running. The runs state is controlled from this button, as well as the external run input. Either of them can start and stop the sequencer. This button is modeled on the run button in "Clocked", from Impromptu Modular.

**Scroll mode** button enabled scrolling while running.

**CV** is the pitch CV output.

**G** is the gate output. When it is active the green LED lights.

**Clk** is the eternal clock input.

**Rst** is the reset CV. It is quite compatible with the reset out of "Clocked".

**Run** CV input. Meant to be hooked up to the Run output of Clocked, or similar master clock.

## Piano roll

It works like you would expect.

Note that there is a blinking "DOS cursor". It does not follow the mouse cursor much of the time. Note editing may done via the keyboard and this cursor, or with the mouse.

The computer keyboard is the fastest way to ender notes. Details are in [Keyboard Summary](./keymap.md).

Conventional editing with a mouse is also fully supported. Details are in [Mouse Summary](./mouse.md).

## What works now

Plays sequence looped.

Edit note attributes (pitch, start time, duration) with mouse or keyboard.

Enter new notes from the keyboard.

Draw Piano roll.

Undo / Redo. Every operation that changes the note data may be undone.

Copy/Paste (you can even copy from one instance and paste to another);

There is a single track, and it is monophonic.

## What doesn't work

There are obviously many missing features. It looks pretty rough.

Note editor things that don't work:

* Time units are always 1/16 notes.
* Insert note is always 1/4.

It is probably possible to crash it with some note editor operations. But please report such things to me.

## Extending the length

There is a temporary hack to make it possible to lengthen a track. You may move the cursor past the end of the track. If you insert a note there, the track will be extended in units of 4/4 bars to accommodate the new note.

Note that it is not easy to know how long your track actually is. And it is currently impossible to shorten it, once you have lengthened it.

## Some hints with editing

Once you have selected a note, or group of notes, it is very easy to move them around, typically by Pressing 's', 'd', or 'p' to set the editor editing start time, duration, or pitch. Then all selected notes may be adjusted with the '+' and '-' keys (and may others).

New notes are inserted by moving the cursor to the desired location, and pressing 'Ins'. There are many other keys for inserting different lengths of notes.