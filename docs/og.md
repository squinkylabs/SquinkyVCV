# Organ-Three

Organ-Three is a polyphonic "Organ" module based on the Hammond tone-wheel organs. Most of the features are features available on Hammond organs, although we have added a couple features of our own.

Although it may be used as a VCO, Organ-Three is a full polyphonic synthesizer voice, meaning it does not need to be patched with VCAs, envelope generators, or filters.

Organ-Three is not intended to be a close emulation of a real Hammond. There is no attempt to model imperfections in the tone-wheel teeth, the slightly out of tune scale, exact key click, and such. But the sound is definitely similar to a Hammond.

For those not familiar with old organs, you can think of Organ-Three as a polyphonic synth voice that uses additive synthesis. But the tuning of the partials is constrained to an approximation of the Hammond tuning, and the envelopes are usually fast attack, fast release, full sustain. Internally Organ-Three has 144 sine-wave VCOs and 32 ADRSs.

Like a real organ, Organ-Three sounds really good when processed by a "Leslie" effect. The Surge Rotary is a very good one, although there are probably others.

Make sure to use the tooltips to find out what the controls do. The panel labels are rather sparse, but the tooltips are very detailed.

## TL;DR getting started

Patch a polyphonic pitch CV to the V/Oct input, patch an appropriate polyphonic gate to the gate in, and try playing some of the presets available on the context menu. For a quick listen, hook Organ-Three directly the the VCV CV-MIDI Module get get a V/Oct and Gate signal, path the Out to the VCV Audio-8.

For use as a general VCO, read the rest of the manual.

## Organ basics

The Hammond organs use a primitive form of additive synthesis. There are nine sine-waves for each key (or voice) that are fixed in tuning to approximate the lower harmonics of a musical tone (and some sub-harmonics). The controls for the volume of these sine-waves are called drawbars. Mixing the tones in various ratios allows the organ to produce different timbres.

The Hammond uses a color coding to classify the drawbars. While the Hammond uses brown, black, and white we used "Squinky Blue" instead of brown. The blue drawbars are sub-harmonics, the white are the fundamental and octaves, and the black one are for non-octave harmonic.

 The drawbars are named after organ "footages", where the length of an organ pipe would determine the pitch. 8' is considered the "fundamental" and is at pitch C4. So the drwbars are, from left to right:

* **16'**, blue. Sub-octave, "Bourdon". One octave below the fundamental. C3.
* **5 1/3'**, blue. third harmonic down an octave, "Quint". G4.
* **8'**, white. fundamental, "Principal". C4.
* **4'**, white, second harmonic, "Octave". C5.
* **2 2/3'**, black, "Nazard". Third harmonic. G5.
* **2'**, white, "Block flöte". Fourth harmonic, C6.
* **1'**, white, "Siff flöte". Fifth harmonic, C7.

## Non-Hammond features we added

The major "non Hammond" feature we added are the two knobs to control the attack and release of the envelopes. While this is not at all authentic, it does allow you to approximate the sound of the traditional "swell" pedal. And it allows an expanded palette of sounds without going too far away from the original.

The percussion section is slightly more versatile that the Hammond percussion. A real Hammond has four switches to control percussion: on/off, normal/soft, fast/slow, third/second. Organ-Three replaces on/off, normal/soft and third/second with the percussion volume controls. One is the volume for the second harmonic, the other is the volume for the third harmonic.

And we did not emulate the way Hammond percussion "steals" tones from the drawbars.

**VCO mode.** If you do not connect the gate input, Organ Three will output polyphonic waveforms all the time, like a VCO. Then you must provide your own volume and timbre controls.

There are CV inputs for each of the drawbar volumes.

## About the presets

Organ-Three ships with a selection of "factory presets". These can be found on the context menu of Organ-Three. These presets mostly come from various internet sources for Hammond organ presets.

We have categorized them by name, so the first word should say what the preset is:

* **basics.** These are commonly recommended starting points for various genres of pop, rock and jazz.

* **songs.** These are presets that are supposed to approximate the sound of various well known Hammond songs.

* **strings, reeds, etc.** These are setting that are supposed to mimic the sounds of other organs. In most cases they sound little like the instruments they are named for, as organs can't really synthesize these instruments well.

Most of the presets have the attack and release set to zero. Many would probably sound "better" with some adjustment there.

## About the CV inputs

First of all, other than V/Oct and Gate, the CV inputs are "monophonic" which is not strictly speaking proper. What this mean is that a signal applied to any of the drawbar inputs or envelope inputs will affect all "voices" equally. Probably the proper way to describe this is that Organ-Three is "polyphonic", but not "polytimbral".

The V/Oct input will determine the number of voices that Organ-Three plays, Organ-Three will play as many voices as there are channels on the V/Oct input, or one voice if V/Oct is not patched.

The CV inputs for the nine drawbars work as follows: the actual drawbar volume will be what is on the knob multiplied by input voltage (and re-scaled). So if a drawbar is set to 0, it will not sound, no matter what the CV is. And conversely, if the CV is zero the drawbar will not sound.

The CV inputs for A and R do not work like this. These CV are added to the knob values. So you can get the full range of attack and release from the CV no matter what the knob setting.

## Reference

**Nine Drawbars.** The nine sliders in the middle of the panel are a mixer of different harmonics and sub-harmonics. The respond more or less like the drawbars on the Hammond. Moving them one number will be about 3db of volume change. At a certain point increasing one may decrease all the others. This Hammond feature makes it easier to mix the drawbars.

**Percussion Volume** There are two knobs at the upper right that control how much of the "percussion" sound will come through. There is one knob for the second harmonic percussion, and one knob for the third harmonic percussion. On a normal Hammond this is an either/or choice - here you can mix both of them.

**Percussion Speed** Controls what we would call the "decay time" of the percussion envelopes. There are two settings - fast and slow.

**Attack and Release knobs** Controls the attack and release time of the main envelope generators. On organs both of these are usually very fast, but an organ usually has a "swell" pedal to allow fade in and fade out. Here it's easier and more versatile to just have attack and release controls.

**Key Click** On/off switch. Mechanical organs often made an audible click when keys are pressed or released. The click is produced by instantly gating a sine-wave on or off, similar to the way a bad mixer will pop when you mute it. When keyclick is on we set the envelopes as fast as possible, when off we slow them down just enough so they don't click.

If you set the attack and release to something slow you will not be able to get a key click.

**V/Oct** input. Polyphonic CV that sets the pitch of each voice.

**Gate** input. Polyphonic gate input that allows each voice to sound. Typically you would want the Gate signal to have the same number of polyphonic channels as the V/Oct input. If nothing is connected to the Gate input, the Organ-Three will sound constantly. We call this **VCO mode**. With VCO mode you would usually put some processing after Organ-Three and then a VCA to let the notes stop.

**Drawbar CV inputs**. Each of the nine inputs can take a control voltage to modulate the level of that drawbar. Refer to the section [About the CV inputs](#About-the-CV-inputs) for details.

**A and R inputs**. These inputs allow a CV to modulate the envelope attack and release. Please look at [About the CV inputs](#About-the-CV-inputs) for information on using these inputs.
