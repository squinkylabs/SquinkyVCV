# Portable sequence format
Entrian, Impromptu Modular, and SquinkyLabs have developed a proposed standard for representing note sequences on VCV's clipboard. This allows moving a common subset of sequence data between modules. All three devs have experimental/test builds of their sequencers that support this proposal.

The format is biased towards data that is similar to what might be in a MIDI file – a series of notes with pitches, start times, and durations. Possibly polyphonic. It can be used usefully in modules that don't use that model, however.

At the highest level, the data is a UTF-8 string containing JSON encoded data. VCV is already using this encoding, so the new proposal will not conflict with VCV's existing clipboard code.

Before diving into details, here is a simple example:

```json
{
  "vcvrack-sequence": {
    "length": 4.0,
    "notes": [
      {
        "type": "note",
        "start": 0.5,
        "pitch": -4.47034836e-8,
        "length": 0.850000024
      },
      {
        "type": "note",
        "start": 1.5,
        "pitch": 0.166666627,
        "length": 0.850000024
      }
    ]
  }
}
```

The top level entity is an object. The property names on that object are the names of different clipboard formats. This document describes the contents of the object named **_vcvrack-sequence_**, but other interchange formats may defined, and an application may put an object on the clipboard with more than one top-level format property. This is similar to the way clipboards work in other applications. For example, a word processor will typically put its native format on the system clipboard, as well as HTML and plain text.

The **_vcvrack-sequence_** has two properties: **_notes_**, and **_length_**. Both of these are required.

**_length_** is a number representing the entire time duration of the sequence. In **_vcvrack-sequence_** time is always representing with metric time, where 1.0 is the duration of a quarter note. Real numbers are used, so time can be finer than just quarter notes. **_length_** is the duration of the whole clip.

**_length_** must be large enough to contain the notes in the **_notes_** section, but may be longer if the sequencer wishes to represent a clip that is "bigger" than just the notes contained in it. For example, an entire sequence might be exactly 16 quarter notes long, but the last note won’t necessarily extend all the way to the end.

**_notes_** is an array of **_note_** objects. The **_note_** objects in the array must be listed in time order. More on this later.

**_note_** objects have four required properties, and two optional ones. **_type_** is required. It is a string property,  and the value is always **_note_**. **_start_** is the start time of the note. Like all time properties it is a real number, where 1.0 is one quarter note. **_pitch_** is a real number, and uses the VCV rack standard of one volt per octave, with 0 being C4. **_length_** is the note length or duration, where again 1.0 is the length of a quarter-note.

Is many cases the first **_note_** will have a **_start_** of 0.0. But this need not be the case. For example, in some sequencers a user is able to select a whole bar and copy it. In this case, the **_start_** of the first note will be its start time relative to the start of the bar. In other words, the duration of the entire clipboard will usually be the duration of the source selection, and all **_start_** properties will be relative to the start of the selection.

**_note_** objects also have two optional properties: **_velocity_** and **_playProbability_**. These properties may be set on note objects, or they may not. Parsers should be prepared to handle either case. **_velocity_** has a range of 0 to 10. **_velocity_** might be patched to a VCA to control volume, but it could be used for anything. Think of it as a unipolar CV that has a constant value for the duration of a note.

**_playProbabilty_** has a value of 0 to 1, and represents the probably that the note in question will play.

## A note on polyphony

It is possible that a module will put a polyphonic sequence clip of the clipboard. This will produce notes that overlap. A consuming module might not be polyphonic, and will have to decide how to interpret these notes.

## Putting other data on the clipboard

While this document specifies a schema value of the **_vcvrack-sequence_**  property, the proposal allows for other top-level properties besides **_vcvrack-sequence_**. We expect that it will be common for some modules to put **_vcvrack-sequence_** information on the clipboard, but also put their own format on the clipboard as well. The reader of this clipboard may then pick what data to parse.

Modules are free to write extra formats like this, however it would be bad for two modules to use the same property name but have different meanings. For this reason we suggest that you use your developer name as a prefix, or even your module slug. For example:

```json
{
    "squinkylabs-stuff": "hello",
    "vcvrack-sequence": {
        "notes": [ ],
        "length": 1
    },
}
```

A module may put anything it likes on the clipboard this way. With the obvious restriction that it must be well formed JSON in a UTF-8 string.

## Best practices for implementations

### Writing to the clipboard

Write all the required properties, even if your module may not user them. For example, you are required to set a length on each note. If your sequencer doesn’t  have a length for a note, put in some plausible value like 1.

Don’t write optional properties if your module can’t provide a valid value. For example if your sequencer doesn’t have a concept of **_velocity_**, don’t write out 5.0. Just omit it and hope the reader knows a reasonable default.

If you write your own proprietary format to the clipboard try to also write a standard format so that other modules that don’t know about your format can get some useful information.

### Reading from the clipboard

There are at least two issues every dev will need to consider. The first is making sure to correctly parse a well-formed **_vcvrack-sequence_** when optional properties are present and when they are not.

The other is what to do if there is malformed data on the clipboard. Most devs will probably not want their module to crash if bad data is pasted into it. Luckily the JSON parser in VCV is pretty robust and difficult to crash. A few ideas are:

* Make sure you can handle the case where the JSON parser can’t parse the clipboard. This will be a very common case in the real world. The clipboard might just have text on it.
* If you find a missing required property, you might either ignore it and skip over, or refuse to paste the entire thing.
* When you find something wrong in the clipboard data, it is a nice courtesy to log a message to help others debug their modules. Like WARN(“no notes property in clipboard”) or WARN(“notes is not an array”).

### Documentation

It would be a nice courtesy to users and other developers if modules would document how they interoperate with the portable sequence format. Especially for modules where it might not be obvious how the notes in this format might map the the data structures of other modules.

## Sample code for writing a note

Here is a working code fragment that creates a **_note_** object:

```c++
json_t* Note::toInteropJson()
{
    json_t* rootJ = json_object();
    json_object_set_new(rootJ, "type", json_string("note"));
    json_object_set_new(rootJ, "pitch", json_real(vOct);
    json_object_set_new(rootJ, "length", json_real(lengthInBeats);
    return rootJ;
}
```
