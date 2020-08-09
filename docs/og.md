# Organ-3

Organ-3 is a polyphonic "Organ" module based on the Hammond tonew-heel organs. Most of the features are features available on Hammond organs, we have added a couple features of our own.

Organ-3 is by no means super close emulation of a real Hammond. There is no attempt to model imperfections in the tone-wheel teeth, the slightly out of tune scale, exact key click, and such. But the sound is definitely similar to a Hammond.

For those not familiar with old organs, you can think of Organ-3 as a polyphonic synth voice that uses additive synthesis. But the tuning of the partials is constrained to an approximation of the Hammond tuning, and the envelopes are usually fast attack, fast release, full sustain. Internally Organ-3 has 144 sine-wave VCOs and 32 ADRSs.

It's easy to get started. Patch a polyphonic pitch CV to the V/Oct input, patch an appropriate gate to the gate in, and try playing some of the presets available on the context menu.

Like a real organ, Organ-3 sounds really good when processed by a "Leslie" effect. The Surge Rotary is a very good one, although there are probably others.

Make sure to use the tooltips to find out what the controls do. The panel labels are rather sparse, but the tooltips are very detailed.

## Organ basics

The Hammond organs are a primitive form of additive synthesis. There are nine sine-waves for each key (or voice) that are fixed in tuning to approximate the lower harmonics of a musical tone (and some sub-harmonics). The controls for the volume of these sine-waves are called drawbars. Mixing the tones in various ratios allows the organ to produce different timbres.

The Hammond uses a color coding to classify the drawbars. While the Hammond uses brown, black, and white we used "Squinky Blue" instead of brown. The drawbars are named after organ "footages", where the length of an organ pipe would determine the pitch. 8' is considered the "fundamental" and is at pitch C4. So the drwabars are, from left to right:

* **16'**, blue. Sub-octave, "Bourdon". One octave below the fundamental. C3.
* **5 1/3'**, blue. third harmonic down an octave, "Quint". G4.
* **8'**, white. fundamental, "Principal". C4.
* **4'**, white, second harmonic, "Octave". C5.
* **2 2/3'**, black, "Nazard". Third harmonic. G5.
* **2'**, white, "Block flöte". Fourth harmonic, C6.
* **1'**, white, "Siff flöte". Fifth harmonic, C7/

## Non-Hammond features we added

The major "non Hammond" feature we added are the two knobs to control the attack and release of the envelopes. While this is not at all authentic, it does allow you to approximate the sound of the traditional "swell" pedal. And it allows an expanded palette of sounds without going too far away from the original.

The percussion section is slightly more versatile that the Hammond percussion. A real Hammond has four switches to control percussion: on/off, normal/soft, fast/slow, third/second. Organ four replaces on/off, normal/soft and third/second with the volume controls. One is the volume for the second harmonic, the other is the volume for the third harmonic.

And we did not emulate the way Hammond percussion "steals" tones from the drawbars.

VCO mode. If you do not connect the gate input, Organ-3 will output polyphonic waveforms all the time, like a VCO. Then you most provide your own volume and timbre controls.

CV inputs for the drawbar volumes.

## About the presets

Organ-3 ships with a selection of "factory presets". These can be found on the context menu of Organ-3. These presets mostly come from various internet sources for Hammond organ presets.

We have categorized them by name, so the first word should say what the preset is:

* **basics.** These are commonly recommended starting points for various genres of pop, rock and jazz.

* **songs.** These are presets that are supposed to approximate the Hammond sound of various well known Hammond sounds. 

* **strings, reeds, etc.** These are setting that are supposed to mimic the sounds of other organs. In most cases they sound little like the instruments they are named for, as organs can't really synthesize these instruments well.

## Reference

**Nine Drawbars.** The nine sliders in the middle of the panel are a mixer of different harmonics and sub-harmonics. The respond more or less like the drawbars on the Hammond. Moving them one number will be about 3db of volume change.

**Percussion Volume** There are two knobs at the upper right that control how much of the "percussion" sound will come through. There is one knob for the second harmonic percussion, and one knob for the third harmonic percussion. On a normal Hammond this is and either/or choice - here you can mix both of them.

**Percussion Speed** Controls what we would call the "decay time" of the percussion envelopes. There are two settings - fast and slow.

**Attack and Release knobs** Controls the attack and release time of the main envelope generators. On organs both of these are usually very fast, but an organ usually has a "swell" pedal to allow fade in and fade out. Here it's easier and more versatile to just have attack and release controls.

**Key Click** On/off switch. Mechanical organs often made an audible click when keys are pressed or released. The click is produced by instantly gating a sine-wave on or off, similar to the way a bad mixer will pop when you mute it. When keyclick is on we set the envelopes as fast as possible, when off we slow them down just enough so they don't click.

If you set the attack and release to something slow you will not be able to get a key click.
