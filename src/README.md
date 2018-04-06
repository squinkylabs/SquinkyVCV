# src folder
This folder contains the top-level Module source code. Almost all of our code that is specific to VCV Rack is in this folder.

Usually all of the code the actually generates and processes audio is elsewhere.
## Experimental Modules
We currently have two of them.
### Vocal Animator
This is a recreation of a circuit invented by Bernie Hutchins, and published in Electronotes magazine in the late 70's.

There are four bandpass filters, roughly tuned to some typical vocal formant frequencies: 522, 1340, 2570, and 3700 Hz. The filters are run in parallel, with their outputs summed together.

The first three filters are modulated by an LFO comprised of 4 triangle wave LFOs running at different frequencies. They are summed together in various combinations to drive each of the filters.

The Mod Rate control shifts the speed of all 4 LFOs while maintaining the ratio of the frequencies.

The filter frequency control moves the first three filters, but not by equal amounts. The lowest filter moves at 1V/Oct, but the middle two move less. The top filter is fixed at 3700 Hz.

The Q control does just what it says - controls the Q (resonance) of the filters.

The Modulation Depth controls how much of the summed LFOs get to each filter. Again, the lower filters move farther, and the top filter is fixed.

The smaller knobs next to the main knobs are attenuverters. They scale and/or invert the control voltage inputs next to them. All the way up and the full CV comes through. As they are turned down less CV comes through. Straight up none passes. As they are turned down further the CV comes back, but inverted.

There are three LFO outputs next to the blinking LFOs. 

Bass boost switch. When it’s on there should be more bass. This is done by switching some or all of the filters from bandpass to lowpass.


LFO Matrix switch. When it’s up the LFOs are closely correlated. In the middle we try to make them a little bit more independent. When it’s down they will often go in different directions.

Other than the ugly panel, we believe this module is close to being ready for release.
### Singing Filter
Like the Vocal Animator, this is a filter bank tuned to the formant frequencies of typical singing voices.

The Fc VC/control moves all the filters up and down.

The vowel control smoothly interpolates between 'a', 'e', 'i', 'o', and 'u'.

The model control selects different models: bass, tenor, countertenor, alto, and soprano.

The brightness control gradually boosts the level of the higher formants. When it is all the way down, the filer gains are set by the singing models in the module, which typically fall off with increasing frequency. As this control is increased the gain of the high formant filters is brought up to match the F1 formant filter.


