# User Keyboard mapping

Any of the keyboard assignments in Seq++ may be changed by the user. Anything is possible from just changing a few keys, to completely changing every aspect of the keyboard interface.

## Basic concepts

A KeyMapping describes how keys should be handled. One of the most important things in a KayMapping is an array of KeyBindings.

A KeyBinding relates a specific key on the keyboard to an action to be performed.

The data for a KeyMapping is stored in a JSON file. JSON files are probably familiar to VCV users already, as much of the VCV Rack configuration is already stored in JSON files.

The data format for Seq++ KeyMapping files is modeled after the analogous feature in the Visual Studio Code editor.

Seq++ defines the default keyboard mapping in a file called *seq_default_keys.json* which is located in the SquinkyLabs/res folder. This file is not user editable, but it is a good example of how to write a KeyMapping file that implements an entire user interface.

You may create an override file in the VCV Rack folder. This file must be called *seq_user_keys.json*

Here is the built-in [seq_default_keys.json](../res/seq_default_keys.json)

## Editing JSON files

Any text editor may be used for editing JSON files, but we recommend you use one that understands JSON. An editor like that will tell you when you have made a basic mistake, and will use syntax coloring to indicate the different parts of the JSON.

We use Visual Studio Code, although there are many alternatives. The built-in text editor in Ubuntu Linux is decent for this.

If you are unfamiliar with JSON, it may take some trial and error to get your key mappings to work. It isn't important how you indent your files and whether you put things on different lines. Note how the examples in this document tend to use "normal" JSON indenting and line breaking, but the actual [seq_default_keys.json](../res/seq_default_keys.json) has very few line breaks or block indents. We to that to make the files more readable.

## Comments in JSON

If you have worked with JSON before, you probably know there is no legal way to put a comment into a JSON file. But there are some tricks you can use if you want. In our file we make use of the fact that extra properties in a Binding will be ignored, so we sometimes add a dummy property called `comment`, like this:

```json
  {"key": "numpad_add", "action": "value.increment.normal", "comment": "edit commands"},
```

## JSON syntax for key mapping

### KeyMapping

The KeyMapping files describe a JSON object. The built in KeyMapping syntax is very simple. The user override file has additional options.

KeyMapping = {
    "bindings": [
        binding1,
        binding2,
        bindingn
    ],
    "ignore_case": [
        KeyCode1,
        KeyCode2
    ]
}

So a default KeyMapping has an array of bindings and an array of KeyCodes whose case we wish to ignore.

### Binding

```json
Binding = {
    "key": "some+key",
    "action": "some.action"},
}
```

We see that a Binding is an object with a Key to be bound, and the name of the action to be performed when that key is activated.

Key = KeyCode(+alt)(+ctrl)(+shift)

A KeyCode is a string, as is an action name.

Here is an example binding:

```json
  {"key": "q+shift", "action": "insert.quarter"}
```

here the Key is "q+shift", and the ActionName is "insert.quarter". Thus capital q will insert a quarter note. At the end of this page is a complete reference of the key names and action names that may be used.

### Ignore case

Ignore case is an optional section that lets you specify keys that should to the same thing regardless of whether they are pressed along with the shift key. For example, in the default keyboard mapping cursor up moves up regardless of the state of the shift key. So we do this:

```json
{
    "bindings": [
        {
            "key": "up",
            "action": "move.up.normal"
        }
    ],
    "ignore_case": [
        "up",
        "down",
        "left",
        "right"
    ]
}
```

Note that `ignore_case` is purely a convenience feature. The following is a completely equivalent way to do the same thing without using `ignore_case`:

```json
{
"bindings": [
    {
        "key": "up",
        "action": "move.up.normal"
    },
    {
        "key": "up+shift",
        "action": "move.up.normal"
    }
]
}
```

### Keys, in detail

As we mentioned above, a Key is a KeyCode with optional modifiers like `shift`. The KeyCode and the modifiers must be separated by a `+`. They may occur in any order.

The allowed modifiers are:
    * `shift`
    * `ctrl`
    * `alt`

Note the on the Macintosh we follow that standard convention that `ctrl` is the `Apple` or `Cmd` key, and the actual `ctrl` key is unused.

Seq++ uses the same key codes as Visual Studio Code, which are:

```
f1-f19, a-z, 0-9
`, -, =, [, ], \, ;, ', ,, ., /
left, up, right, down, pageup, pagedown, end, home
tab, enter, escape, space, backspace, delete
pausebreak, capslock, insert
numpad0-numpad9, numpad_multiply, numpad_add
numpad_subtract, numpad_decimal, numpad_divide
```

In Seq++, they key codes always refer to the physical keys on a US keyboard. So in Seq++ the `a` key is the key on the left of the middle row, just to the right of the Caps Locks key. Even though on a French keyboard this key is called `q`.

You will note also that the number key are completely separate from the numeric keypad. This means that if you can the same function to happen on the `2` key as well as `numpad2`, you must put in two KeyBinding objects, one for each physical key.

### Other JSON properties, besides bindings

These properties are only valid in your *seq_user_keys.json* file - the are no, and can not, but present in the built-in mappings.

*use_defaults* If this optional property is false, then only your keymap file will be used, and the contents of the built-in key mapping will be ignored. If it is true, or not present, then all keys will first go to your mapping, but it a key is not bound in your mapping, then it will be sent to the default mapping.

*grab_keys* If this optional property is false, then Seq++ will not try to grab the cursor keys, or any other keys, away from VCV Rack. If you plan on using your own mappings to move the cursor, and your mappings do not use keys that VCV Rack uses, then you should set this to false so that VCV can process keys as it would like to.

## Some suggestions

Start with a very minimal *seq_user_keys.json*, and build it up a little bit at a time. The minimal file that will work just needs to have the *bindings* array with one binding in it.

Run VCV Rack from the command line, and look carefully at the console output as you load an instance of Seq++. Debugging information will be output to the console, and more importantly detailed errors will be logged to the console.

If you are going to make a lot of changes, it will be worthwhile to plan it first. There are a lot of commands to move the cursor and insert notes, so it is very easy to run out of convenient keys to use for all the functions. If you don't plan everything ahead of time, it's likely you will code yourself into a corner where there are no good keys "left over" for the remaining actions.

## Actions reference

| Action name | Effect |
| --- | --- |
|help | Open the manual in a new browser |
|loop | Toggle sub-range loop on and off |
| change.track.length | Set track length to cursor location |
| insert.default | Insert the default note at current pitch/time |
| insert.whole.advance | Insert whole note and move cursor after it |
| insert.half.advance |  Insert half note and move cursor after it |
| insert.quarter.advance |  Insert quarter note and move cursor after it |
| insert.eighth.advance |  Insert eighth note and move cursor after it |
| insert.sixteenth.advance |  Insert sixteenth note and move cursor after it |
| insert.whole | Insert whole note, don't move cursor |
| insert.half | Insert half note, don't move cursor |
| insert.quarter | Insert quarter note, don't move cursor |
| insert.eighth | Insert eighth note, don't move cursor |
| insert.sixteenth | Insert sixteenth note, don't move cursor |
| move.left.normal | Move cursor left one grid unit |
| move.right.normal | Move cursor right one grid unit |
| move.up.normal | Move cursor up one semi-tone |
| move.down.normal | Move cursor up one semi-tone |
| move.left.all | Move cursor to the start of the first bar |
| move.right.all | Move cursor to the last bar |
| move.left.measure | Move the cursor left by one bar |
| move.right.measure | Move the cursor right by one bar | |
| move.up.octave | Move the cursor up one octave |
| move.down.octave | Move the cursor down one octave |
| select.next | Select the next note after the currently selected one  |
| select.next.extend | Select the next note, while leaving others selected |
| select.previous | Select the previous note before the currently selected one |
| select.previous.extend | Select the previous note, while leaving others selected |
| select.all | Select all the notes |
| value.increment.small | Add a semi-tone or 64th note  |
| value.increment.normal | Add a semi-tone or a grid unit |
| value.increment.large | Add an octave or four grid units |
| value.decrement.small | Subtract a semi-tone or 64th note |
| value.decrement.normal | Subtract a semi-tone or a grid unit |
| value.decrement.large | Subtract an octave or four grid units |
| cut | Cut |
| copy | Copy |
| paste | Paste |
| edit.start.time | Set the editor to edit note start times |
| edit.duration | Set the editor to edit note durations |
| edit.pitch | Set the editor to edit note pitch |
| grab.default.note | Use the note under the cursor as the default insert note |
| delete.note | Delete the note under the cursor |
