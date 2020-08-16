# Basic VCO

## About the waveforms

In some cases we have provided two different implementations of the same waveform: a "basic" one that is very pure and very fast, and a "clean" version that is less fast, and extremely pure. In all cases the basic version is comparable with the best competing VCOs. What artifacts/distortion are in the basic waveforms are probably inaudible. But the "pure" versions have much less distortion.

**Sin** This is a very clean sin wave. It is based on the sin part of Fundamental VCO-1, which is an efficient and pure implementation. The waveform is not perfect, but the main artifact it has is a very low level of harmonic distortion. Like many of these waveforms, we don't know if there is any audible difference between this and a "perfect" sine.

**Triangle** This is a very typical simple triangle wave generator, used by several other VCOs. It is a "naive" implementation with no attempt at alias reduction. That is because the trinagle wave harmonics drop off so quickly that the alias products are probably inaudible.

**Saw** This is the same saw as VCO-1, and out own Demo VCO3. It uses "MinBLEP" to get rid of almost all of the aliasing. There is a very small amount of aliasing in the top octave, but again very small. It does have the correct good harmonics in the top octave, so it should not sound dull.

