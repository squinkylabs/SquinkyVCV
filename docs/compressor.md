# Compressor

## Notes for testers

Afaik everything here works. In general the knob ranges and feel are intentional. The compression ratios included are the only ones I have planned for now. It seems reasonably CPU efficient.

Some outstanding issues:

* It might be nice to have a gain reduction meter.
* I think the dry signal gets makeup gain applied. But this seems wrong.
* The wet/dry knob feels strange, since the "wet" is always quieter than dry. See previous item.
* Could probably use some more compression ratios.
* The soft knee is 12 db wide. I could adjust that or make multiple settings (soft, medium, hard?).
* Should there be a second input/output for "stereo" operation?
* Should the module be bigger (esp. if we add more features)?

## About compressor

It is a very normal compressor / limiter. Most of the controls should be very familiar.

It is fully polyphonic, although all channels will have the same settings.

The "Limiter" is an infinite ratio hard-knee compressor. It is also implemented much differently than the other ratios. Consequently it uses almost no CPU when processing 16 channels. So if you want to "Squash everything" it's a very economical setting.

It had fixed (preset) compression ratios and knee width. Using presets like this allows me to have very high audio quality without using much CPU.

There is one crazy feature - the "reduce IM distortion" switch. This switch selects different topologies of the gain detector. When it is off the compressor is "normal"; when it is on there is a little bit less distortion, and the attack / release characteristics are changed a little bit.
