# Xforms

The Xforms (short for transforms) are a collection of  “destructive edit operations.” They all operate over the selected notes, often transforming a note at one pitch to another pitch.

They all work more or less the same way:

1. Select some notes as you normally would in the note grid.
2. Pick one of the xforms from the context menu.
3. A dialog will come up with settings for that xform. Set them as you like.
4. If you press OK, the xform will by applied to the notes you selected.

Many of the xforms require you to select a scale. The scale is stored in the patch, so once you set it for your patch it should be remembered and you shouldn’t have to re-enter it every time.

## A note about scales and harmony

Many of the xforms have a "Relative to scale" selection, and when selected you may choose one of the scales.

When Relative to scale is not selected, all xforms will treat pitch in absolute semitones. So, for example transpose +2 will raise each pitch by a whole tone.

When Relative to scale is selected, all pitches are treated as degrees of the scale. All pitch Xforms will map notes of one scale degree to a different scale degree.

One effect of this is that notes that are in  the current scale will stay in the current scale after being xformed. Notes that are not in the current scale may or may not end up in the scale after xform.

If you select one of the diatonic scales, the resulting operations will probably be familiar or intuitive. For example, transposing by a third gives a familiar harmony, where every note is raised by a major or minor third, depending on the scale degree.

If one of the other (non-diatonic) scales is selected, like diminished, the results of the Xform may be more mysterious. Note that with the minor pentatonic scale, the first step is a minor 3rd.

The only thing Seq++ knows about scales are the notes that are in them. It does not know how each degree might function in a harmony. It knows nothing about melodic direction, so it cannot implement a scale like melodic minor that has different notes depending on whether the pitch sequence is rising or falling.

## Scales

Seq++ had a fairly simple concept of a scale. All scales in Seq++:

* Are made up of pitches from the 12-note even tempered chromatic scale.
* Repeat every octave.

Other than that, there are no limits. Could Seq++ have a scale made up of C,C#,D,D#,E and no other notes? Sure it could.

## Steps

When "relative to scale" is selected, xforms will ask you to select octaves and steps. But what is a step? One step is the distance from one scale note to to the next. So in the key of C, transposing F up by one step will move it to G, since G is the next step after F. In this case the step is a whole tone. But transposing E up a step will transpose it to F, a semi-tone.

With diatonic scales, you can think of steps as intervals with a different name. Zero steps is unison. One step is a second, two steps are a third, etc.

But let's consider the minor pentatonic. In Em pentatonic, if you transpose E up a step it goes to G. But in this case it's three semi-tones. So in any scale in Seq++, there is always a "step" from one scale note to the next. In diatonic scales that's one or two semitones, but since Seq++ supports other kinds of scales, the number of semitones in a step could be almost anything.

## Transpose

If Relative to scale is not checked, it will be a normal transpose that adds or subtracts some number of half-steps to the pitch of the selected notes.

If Relative to scale is selected, it will do something very much different. The current scale will be shown, and may be changed. Notes that are in the selected scale will be transposed by the number of scale steps you enter, and end up still in the key. This means that the transpose amount will not always be the same in number of semitones.

The UI will change when you select Relative to scale, as it must switch between semitones and scale degrees.

## Invert

The pitch is inverted around an axis that you select. This turns rising melodic lines into descending, and all sorts of other things. Notes that are above the pitch axis will flip to be below the axis, and vice versa. The formula is very simple:   new pitch = 2 * pitch axis - old pitch.

If Relative to scale is selected, then all flipping will be done by scale degree, not by absolute semitone pitch.

Example: in C Major, select C4 as pitch axis. C4, D4, E4 would get inverted to C4, B3, A3. So the formula for “in key” invert is  (new scale degree) = (2 * axis scale degree) - (original scale degree).

## Reverse Pitch

The order of the pitches is reversed, but the start times and durations are preserved. In this way it is similar to the classic “retrograde,” but only applied to pitch.

Simple example: If you start with C, E, G, F, F and reverse it you will get F, F, G, E, C.

## Chop Notes

This will chop a single note into multiple notes. By default all the notes are at the pitch of the original note, but there are cool options for doing trills or arpeggios if you would like.

The main setting is "notes," which determines how many notes will be generated by chopping a single note. For example, chopping a quarter note into 4 notes will generate 4 sixteenth notes.

The "ornament" setting determines the pitch of the notes after they are chopped:

* None - generated notes will be the same pitch as the original note.
* Trill - generated notes will alternate between original pitch and a transposed pitch.
* Arpeggiate - generated notes will keep rising or falling, depending on the sign of the "steps" setting.

Typical uses

* Insert a triplet. Select a quarter note. Invoke chop notes. Leave ornament set to “none,” notes = 3. Your quarter note will become an eighth note triplet.
* Ratchet. Pick some eight notes from a sequence of eight notes. Set ornament = none, notes = 4. The selected eights will each become four thirty second notes, for a typical "ratchet" effect.
* Normal trill. Select a long note, invoke chop notes. Set ornament to trill and set steps to 1. You will get a trill going up a semitone.
* Generate arpeggiated chord. In CMaj select a long C note. Invoke chop notes. Set ornament to arpeggiate, check "Relative to scale," and select steps to 4 and notes to 4. The single C note will turn into a major arpeggio of c, e, g, b.
* Jumping octaves. Chop a note using the trill setting, but set the steps to make an octave. Now the "trill" will be a sequence of notes going up and down an octave.

Note that to chop the notes, Seq++ need to interpret the notes. It needs to know that a note whose duration is 85% of a half note is actually a half note. So if you use 100% articulation you will have no surprises. If the articulation drops below 50% (staccato), then you may get unintended results.

## Quantize pitch

This one is very simple - moves all the notes that are not already in the scale to the closest scale note. In case of a tie it picks the lower note.

## Insert Triads

It always inserts triads based on the current scale. If a selected note is not in the scale, it will not get turned into a triad.

The first three triad types, root position, first inversion, and second inversion are all pretty simple. The selected note will become the root note of a triad in the selected scale. In root position the third and fifth will be above. Ex: c4, e4, g4. In first Inversion they will be e3, c4, g4, and in second inversion g3, c4, e4.

In auto, the inversion will be selected automatically. The root note will always stay at the original pitch, but the third and fifth might be moved an octave lower.

The auto algorithm is pretty simple: it prefers to move all the notes as short a distance as possible, and it likes to avoid moving all three notes in the same direction.

Auto 2 is a little crazy. It has the same criteria as Auto, but it can move any of the notes up and down and octave if it makes it “better.” This tends to reduce the amount of parallel motion in the chords, but may move them to unexpected octaves.

The intention of the auto mode is to generate voice leading that is better than you might get by putting all chords in root position. If you are good at voice leading you can surely do a better job yourself. But we hope it is fun in any case.
