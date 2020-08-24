# Basic VCO

Basic VCO is yet another tinny VCO - in this case it's 3HP wide. Like most tinny VCOs, it can only output one waveform at a time. This can be an advantage, however in that it lets Basic VCO use less CPU than it might otherwise. The significant properties of Basic VCO are:

* It has all the standard waveforms, plus the "Even" waveform.
* It has almost no aliasing.
* It has almost no DC on the output.
* It uses less CPU than most other VCOS, sometimes dramatically less.
* It has three pitch knobs to speed patching: octave, semitone, and fine.
* It has full PWM implementation.
* It has a dedicated exponential FM input for modulation.
* Attenverters on FM and PWM inputs make modulation much easier.

We were actually surprised by how many features we were able to jam into this slim module.

## About the waveforms

In some cases we have provided two different implementations of the same waveform: a "basic" one that is very pure and very fast, and a "clean" version that is less fast, and extremely pure. In all cases the basic version is comparable with the best competing VCOs. What artifacts/distortion are in the basic waveforms are probably inaudible. But the "pure" versions have much less distortion.

**Sin** This is a very clean sin wave. It is based on the sin-wave we designed for Kitchen-Sink. The waveform is not perfect, but the main artifact it has is a very low level of low-order harmonic distortion. Like many of these waveforms, we don't know if there is any audible difference between this and a "perfect" sine.

**Triangle** This is a very typical simple triangle wave generator, used by most other VCOs. It is a "naive" implementation with no attempt at alias reduction. That is because the triangle wave harmonics drop off so quickly that the alias products are probably inaudible.

**Saw** This is the same saw as our own Demo VCO3, which is based on the Fundamental VCO-1. It uses "MinBLEP" to get rid of almost all of the aliasing. There is a very small amount of aliasing in the top octave, but again very small. It does have the correct good harmonics in the top octave, so it should not sound dull. In this implementation we got rid of the DC offset

**Pulse** This is also very similar to the pulse wave in Fundamental VCO-1, although it does not have any DC offset, and it does not click when modulated via the PWM input.

**Even** Even is an unusual waveform seen on very few VCOs. It is of course in Befaco's EvenVCO, and in our copy, EV-3. The special sauce of the Even waveform is that it only has even harmonics. All the other "standard" waveforms either have even and odd harmonics, or just odd harmonics. Our implementation is very similar to the Befaco, but is made from parts of the Fundamental VCO-1 so it is more CPU efficient than the originals.

**Pure sin** Another version of the sine-wave. This one has less harmonic distortion that our regular sin, and uses a little more CPU to achieve it. It is actually the sine generator from our EV-3 VCO. The Befaco EvenVCO sine ia also very pure, but it use way too much CPU for its DSP.

**Pure triangle** This is an interesting way to generate a triangle that we first saw in the Befaco EvenVCO. Instead of being a naive direct triangle generator it generates a square wave, uses MinBLEP to get rid of the aliasing, then integrates the square to get a very, very pure triangle. Again, this uses more CPU than our basic triangle.

## Reverence

**octave** knob changes the base pitch of the VCO by octaves.

**semi** knob transposes the pitch in semitones up to plus or minus 12.

**fine** knob tunes the VCO up or down up to a semitone.

**wave** selects the waveform. In order to now what waveform is selected you must use the tooltips (or your ears).

**pw** knob sets the initial pulse width. It only has an audible effect when the waveform is set to square.

**fm** attenuverter scales the fm CV. Straight up is no modulation, fully left is maximum inverted modulation, and fully right is maximum non-inverted modulation. The attenuverters have an "audio taper" to make it possible to control the modulation from subtle to insane.

**pwm** attenuverter scales the pwm CV input.

**pwm** input jack is a polyphonic CV input. If a monophonic source is connected it will control all the VCOs, if a polyphonic source is connected each channel of the pwm input will module a single VCO.

**fm** input jack receives the FM CV.

**v/oct** input jack is where the main control voltage comes in. Basic VCO will set the number of VCOs to match the number of channels on the v/oct input, although if no CV is connected Basic VCO will still run one VCO.

**out** is the polyphonic audio output.
