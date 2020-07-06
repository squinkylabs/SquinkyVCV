# Kitchen Sink

This VCO contains a sinewave oscillator that can do "through zero linear FM", with an ADSR, a wave-shaper, and a bunch of VCAs. If you are familiar with the FM OP VCO, it's a lot like that, with the addition of the wave-shaper and a VCO sync input.

The ability to do FM along with other classic forms of synthesis gives quite a wide palette of sounds that would typically require many more modules to achieve.

The build-in envelope generator can be set very fast, but also it is capable of Moog-like "snap" as an option.

In addition to the sine waveform, the wave-shaper in triangle mode can can morph from triangle to sawtooth,the wave-folder setting generates classic wave-folder timbres reminiscent of the Buchla modules.

The built-in ADSR is lifted from the Fundamental ADSR, but we made it a little faster and added the Mini Moog “snap”. Snap is assumed to be an accident in the old Moog modules, but it gives the envelope more of an AHDSR shape, without requiring finicky settings of the Hold time.

It has octave and ratio. The ration only goes up (x2, x3, x4), since the octave lets you get ½, ¼.

It’s fully polyphonic.

## A pedantic note about FM vs. PM

Most modules and synthesizers that claim to do "FM" really use phase modulation. Even the FX-7 used PM. The advantages of phase modulation of frequency modulation are:

* PM Timbres stay the same as pitch is changed, whereas FM doesn't.
* FM can easily go out of tune when heavy amounts of FM are applied. PM doesn't have this problem.

## Some patch ideas

**Classic wave-folder patch**. Don't use any FM. Set the waveform to wave-folder. Route the ADSR to the "Shape" parameter. As more modulation gets to the "shape" it will fold more, hence get brighter. It sounds a little bit like a swept sync VCO, but not really the same.

Instead of using the build in ADSR for the shape modulation, you may patch anything into the shape input.

**Classic hard sync**. You need two VCOs: carrier and modulator. Kitchen sink will be the carrier, you can use anything for the modulator. Run an output like a square or saw from the modulator to the sync input of the carrier. Set the initial frequency of the carrier above the modulator. Modulator will set the pitch, carrier frequency sort of sets where the harmonics will be. Try sweeping the carrier frequency for all sorts of classic (old school) sync effects.

**FM with other waveforms**. Use the triangle / saw waveform as either the carrier or modulator waveforms in a two operator FM patch. While you are going this, patch some other CV into the shape input for morph it between triangle and saw.

**Morph from Triangle to Saw**. Don't use any of the FM or sync options, but just use it as a VCO with a waveform that morphs from triangle to saw from a CV input.

**Combine features**. There aren't many VCOs that let you use FM, sync, and wave-folding all at the same time. Since all of these features tend to add a lot of harmonics, most attempts to combine them will probably result in a fizzy mess. But there are also many cool sounds lurking in there somewhere. Or if you are looking for a big fizzy mess, go crasy and turn these all up!

## How the knobs, CV inputs, and ADRS interact

Most of these word identically in kitchen-sink. Let's use "Depth" as an example.

If there it nothing patched to the Depth input, and the ADSR button below the depth knob is "off", then the linear FM modulation depth is controlled entirely from the knob.

If a signal is patched into the Depth input, then the knob and the CV together determine the modulation. An easy way to think of this is that the knob is setting the sensitivity of the Depth CV input.

Similarly, if there is nothing patched to the Depth CV, but the ADSR button under the Depth knob is on, then the knob and the ADRS together determine the modulation. An easy way to think of this is that the knob is setting the sensitivity of the ADSR controlling the depth.

If both the ADSR and the CV are active, then the ADSR and the CV are combined, while the knob still controls the sensitivity.

To those more mathematically inclined the knob value, ADSR, and CV are all multiplied together (with some scaling), and the resulting product controls the LFM depth.

Note that all CV input are polyphonic, so individual VCOs can be modulated. If the input is monophonic, however, then it will be copied to all the destinations.

## The controls and input jacks

**Gate CV** Polyphonic gate input CV. The CV in each channel is the gate to the ADSR for that channel.

**V/Oct CV** Polyphonic Pitch (in volts per octave) input CV. The CV in each channel will determine the base pitch of the VCO for that channel. Also, the number of channels present in the V/Oct input will determine the number of active channels or voices. For example, if a four channel CV is patched into V/Oct, the Kitchen-sink will generate four VCOs.

**FM**
**Depth**
**LFM**
****
****
****
****

If you use the sync input, remember that the sync input will be the modulator, and it works best if it is lower in pitch than kitchen sync.


The mod knob controls the pitch modulation depth. Not super useful when using as part of a multi operator FM patch, but useful with the wave-shaper.
