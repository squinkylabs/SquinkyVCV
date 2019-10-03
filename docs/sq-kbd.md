# User Keyboard mapping

Any of the keyboard assignments in Seq++ may be changed by the user. Anything is possible from just changing a few keys, to completely changing every aspect of the keyboard interface.

## Example

## Basic concepts

A KeyMapping describes how keys should be handled. One of the most important things in a KayMapping is an array of KeyBindings.

A KeyBinding relates a specific key on the keyboard with an action to be performed.

The data for a KeyMapping is stored in a JSON file. JSON files are probably familiar to VCV users already, as much of the VCV Rack configuration is already stored in JSON files.

The data format for Seq++ KeyMapping files is modeled after the analogous feature in the Visual Studio Code editor.

Seq++ defines the default keyboard mapping in a file called *seq_default_keys.json* which is located in the SquinkyLabs/res folder. This file is not user editable, but it is a good example of how to write a KeyMapping file that implements an entire user interface.

You may create and override file in the VCV Rack folder. This file must be called *seq_user_keys.json*

Here is the built-in [seq_default_keys.json](../res/seq_default_keys.json)

## Editing JSON files

Any text editor may be used for editing JSON files, but we recommend you use one that understands JSON. And editor like that will tell you when you have made a basic mistake, and will use syntax coloring to indicate the different parts of the JSON.

We use Visual Studio Code for this, although there are many alternatives. The built-in text editor in Ubuntu Linux is decent for this.

## JSON syntax for key mapping

### KeyMapping

The KeyMapping files describe a JSON object. The built in KeyMapping is very simple. The user override file has additional options.

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

So a Binding is an object with a Key to be bound, the name of the action to be performed when that key is activated.

Key = KeyCode(+alt)(+ctrl)(+shift)

A KeyCode is a string, as is an action name.

Here is an example binding:

```json
  {"key": "q+shift", "action": "insert.quarter"}
```

here the Key is "q+shift", and the ActionName is "insert.quarter". Thus capital q will inserts a quarter note. At the end of this page is a complete reference of the key names and action names that may be used.

### Ignore case

Ignore case is an optional section that lets you specify keys that should to the same thing regardless of whether they are pressed along with the shift key. For example, in the default keyboard mapping cursor up moves up regardless of the state of the shift key. So we do this:

```json
{
"bindings": [
    {"key": "up", "action": "move.up.normal"}
],
"ignore_case": [
    "up",
    "down",
    "left",
    "right"]
}
```
Note that `ignore_case` is purely a convenience feature. The following is a completely equilivilent way to do the same thing without using `ignore_case`:

```json
{
"bindings": [
    {"key": "up", "action": "move.up.normal"},
    {"key": "up+shift", "action": "move.up.normal"}
]
}
```
### Keys, in detail