# Xforms 

## Introduction

The Xforms (short for transforms) are a collection of  "destructive edit operations". They all operate over the selected notes, often transforming a note at one pitch to another pitch.

They all work more or less the same way:
Select some notes as you normally would in the note grid.
Pick one of the xforms from the context menu.
A dialog will come up with settings for that xform. Set them as you like.
If you press OK, the xform will by applied to the notes you had selected.

Many of the xforms require you to select a scale. The scale is stored in the patch, so once you set it for your patch it should be remembered and you shouldn’t have to re-enter it every time.

## Transpose

If "Relative to scale" is not checked, it will be a normal transpose that adds or subtracts some number of half-steps to the pitch of the selected notes.

If keep in key is selected, it will do something very much different. The current scale will be shown, and may be changed. Notes that are in the selected scale will be transposed by the number of scale steps you enter, and end up still in the key. This means that the transpose amount will not always be the same in number of semitones.

The UI will change when you select “Relative to scale”, as it must switch between semitones and scale degrees.
Invert
The pitch is inverted around an axis that you select. This turns rising melodic lines into descending, and all sorts of other things. Notes that are above the pitch axis will flip to be below the axis, and vice versa. The formula is very simple:   new pitch = 2 * pitch axis - old pitch.

If “Relative to scale” is selected, then all flipping will be done by scale degree, not by absolute semitone pitch.

Example: in C Major, select C4 as pitch axis. C4, D4, E4 would get inverted to C4, B3, A3. So the formula for “in key” invert is  (new scale degree) = (2 * axis scale degree) - (original scale degree).
Reverse Pitch
The order of the pitches is reversed, but the start times and durations are preserved. In this way it is similar to the classic “retrograde”, but only applied to pitch.

Simple example: If you start with C, E, G, F, F and reverse it you will get F, F, G, E, C
Chop Notes
This will chop a single note into multiple notes. By default all the notes are at the pitch of the original note, but there are cool options for doing trills or arpeggios if you would like.

Some obvious uses: It provides a way to make tuples (although you have to edit the pitches afterwards. Can automatically do “ratchet” effects. The trill setting will make ornamentations.

Note that to do this operation, Seq++ need to interpret the notes. It needs to know that a note whose duration is 85% of a half note is actually a half note. So if you use 100% articulation you will have no surprises. If the articulation drops below 50% (staccato), then you may get unintended results.

## Quantize pitch

This one is very simple - moves all the notes that are not already in the scale to the closest scale note.

## Insert Triads

It always inserts triads based on the current scale. If a selected note is not in the scale, it will not get turned into a triad.

The first three triad types, root position, first inversion, and second inversion are all pretty simple. The selected note will become the root note of a triad in the selected scale. In root position the third and fifth will be above. Ex: c4, e4, g4. In first Inversion they will be e3, c4, g4, and in second inversion g3, c4, e4.

In auto, the inversion will be selected automatically. The root note will always stay at the original pitch, but the third and fifth might be moved an octave lower.

The auto algorithm is pretty simple: it prefers to move all the notes as short a distance as possible, and it like to avoid moving all three notes in the same direction.

Auto 2 is a little crazy. It has the same criteria as Auto, but it can move any of the notes up and down and octave if it makes it “better”.
