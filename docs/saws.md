# Saws
Emulation of Roland Super-Saw.

[add image here]()

## Description of Saws

Saws is intended to be a very accurate emulation of the original found in Roland's JX-8000 synthesizer from 1996. We based our module on the research of Adam Szabo, who analyzed the original for his Bachelor's Thesis in 2010.

While there are several "super saws" for VCV, Saws is the closest to the original, as far as we know.

Saws contains seven sawtooth oscillators that all track the same control voltage. They may be de-tuned, and the mix can be controlled.

The original has an "interesting" approach to alias reduction. It makes no attempt to reduce the aliasing in the high frequencies, but it does have highpass filter tuned to the VCO frequency. This suppresses the really nasty alias tones that are below the fundamental. Combined with the detuned saws, the high frequency aliasing is said to add to the "airy" sound of the the Super-Saw.

The Super-Saw also assigned random phases to the saws each time a note is re-triggered, which prevents successive notes from all sounding the same. This feature, too, is available with Saws.

We have attempted to implement the Super-Saw as closely as possible, and have added no features, other than an oversampling control which is normally off.

## The controls

The pitch is controller by the signal patched into the **V/8 input**.

The **Octave** knob will transpose the VCO in even octaves. The setting is displayed above the knob.

The **Semi** knob will transpose the VCO in semitone intervals. It has a range of 11 semitones up and down. The semitone setting is displayed above the knob as a pitch, such as "G" or "C#".

The **fine** knob tunes the VCO by up to a semitone up or down.

The **FM** knob attenuates the pitch modulation signal patched into the **FM** input.

The **Detune** knob spreads out the pitch of the seven saws. The middle saw stays on pitch, but the other ones move away from it, both higher and lower, as the detune is increased.

There is a Detune CV that allows the detuning to be voltage controlled. As usual the is an attenuverter to scale the modulation.

The **Mix** knob controls the mixing of the detuned saws against the in-tune saw. Then the mix is all the way down, only the middle saw is heard. As with the Detune, the Mix may be voltage controller, and there is an attuenuverter.

Both the Mix and Detune controls were designed to respond just like the controls in the JP-8000.

The **Trigger** input is used implement the phase randomization on new notes that was mentioned above. Any time the Trigger input goes from low to high it will randomize the phase of all the saws.

There is a button in the middle of the panel, which by default is labeled "Classic". This button controls the alias reduction. In the Classic setting we emulate the original - lots of high frequency aliasing and a high pass filter to remove the low frequency aliasing. In the "Clean 1" setting we remove the high-pass filters, and use 4X oversampling to reduce all the aliasing to low levels. "Clean 2" is similar, but increases the oversampling to 16X,

## Other information

Szabo's thesis, which we used to make Saws, can be found [here](https://www.nada.kth.se/utbildning/grukth/exjobb/rapportlistor/2010/rapporter10/szabo_adam_10131.pdf).
