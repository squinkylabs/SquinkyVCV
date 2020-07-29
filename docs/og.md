# Organ

Note: you must hook up the gate input for sound to come out. This module is a full instrument with its own (simple )envelope generators.

Working:

* 16 voice polyphonic.
* All the normal drawbars.
* Percussion (partially).
* Reasonable CPU usage.

Not working:

* No control of percussion decay time.

Known bugs:

* If any of the harmonics get above 40 k the audio goes insane.

Will do:

* Percussion decay time.
* tune the attack and release to make them faster without clicking.
* probably add some control over "key click". Won't sound exactly like a Hammond though.
* CV control of the drawbars, although probably not polyphonic.
* Probably make some presets.
* Will try to reduce the CPU usage.
* Probably a mode where the internal envelope generators are turned off so it acts like a poly VCO, rather than a synth voice.

Could:

* Add pitch modulation input.
* Add tuning controls and/or pitch modulation CV
* Have other envelopes besides "Hammond" ones.
* Have all the drawbar harmonics available as percussion harmonics, too.
