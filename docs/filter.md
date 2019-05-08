
# Filter

The filter module is a work in progress. The documentation is for the parts that are working. Since there is no CV yet you can't really do anything with it.

Filter is yet another ladder filter, but one that combines many of the ladder enhancements that have been discovered over the decades since the Moog filter first came out.

Filter Provides many more shapes that just four pole low-pass. We believe this innovation was first used in the Oberheim Matrix-12, and it is that implementation we have borrowed.

Like the Moog filter, Filter has a pleasant mild distortion to make it sound fat, although it is possible to dial in more and different distortions.

## change log

## next

Reduced overall output volume 6db.

Mode the overall filter much more stable.

Removed the choices for bass makeup gain and went with the standard one. It works now.

Reduced the input gain of the folder to make it less crazy. Asym Folder is still crazy, and will still go into chaotic modes pretty easily.

## 5/5/19

Made drive default to minimum (the old default was quite distorted).

Fixed a bug where high CV would sent the filter into instablility.

Calibrated drive and overall volume to be like Alma.

Played around with the internal gain staging of "Transistor" voicing to more like a normal moog/Alma. Somewhere in between there and where is was before. I will look that some of the other voicings in the future and see if they need work.

Moved the tuning of the filter up by about an octave. Should open up better, although may still be slightly dull compared to other filters.

### 5/4/19

Fixed a terrible mistake where "Drive" was defaulting to 0, which is a very extreme setting.

Made it stereo.

Added all the inputs and controls, although most are still not operational. The panel will look a lot different, but will function the same (for now);

There is a new voicing setting, "Clean", but aside from being clean, it's also where I experimenting with the bass makeup, so the sound there is going to change in the future.

## Important notes

Although the filter can produce many responses besides just four pole lowpass, it it still at its heart a four pole lowpass with some fancy stuff on top. Because of this, many of the responses other than lowpass will work less well the more the filter itself is pushed away from perfection. So with enough *drive*, *edge*, *Caps* and such it will be less and less like a highpass or a bandpass.

Many of the controls change the distortion level or the character of the distortion. So sometimes turning one won't sound that much different from turning another one. Or you keep adding more and more distortion until everything is just a big flabby mess. The Drive, Voicing, and Edge all add or change distortion. When you are first learning to find sounds with Filter, try keeping the distortion to a moderate level so that it's easier to hear what the other controls do.

Also, adding a lot of distortion starts to make the filter act less like a filter. This is particularly true with "extra" filter types. These filter types will only sound as advertised if the Drive is moderate. Also, the Edge and caps controls can make the frequency responose of the extra filter types change a lot. They are all useful sounds, but again, while learning about the different filter types it can be easier if you keep the other controls near their default settings.

## Controls

**Fc** - filter cutoff.

**Q** - filter resonance, or Q. At the moment it won't usually go into self oscillation. I should fix that.

**Type** - filter response type. Choices are four pole lowpass, three pole lowpass, two pole lowpass, one pole lowpass, two pole bandpass, two pole highpass with one pole lowpass, three pole highpass with one pole lowpass, four pose bandpass, one pole lowpass with notch, and three pole allpass with one pole lowpass.

Note that many of these filter type are not what you would normally expect from, say a highpass filter. But they are all useful sounds. Also note that technically when the resonance it turned up, they are all going to have four poles - of course it is impossible to truly make a one-pole resonant filter.

**Drive** - controls the signal level going into the filter. More drive gives more distortion. Too much and the filter stops working right and gets all flabby.

**Voicing** selects different types of distortion. The first selection is the standard one that is pretty close to a Moog. The "Asym" ones have a lot of even harmonics, the others are all odd harmonics. Some of them, like "Fold", will radically affect the sound.

**Edge** - 4P Ladder filters are pretty mellow, even when driven hard, because much of the internal distortion get filtered very heavily. This control changes the internal gain structure of the filter so that it still has the same frequency response, but the spectrum will change. Set in the middle is "normal". To the left the earlier gain stages will be driven harder, and to the right the later stages will be driven hard.

**Caps** - simulates using inaccurate capacitor values. At the minimum setting, the are all perfectly matched. As the value is increased, they go to typical values that would be found in a Moog, up to very imprecise values. That said, the effect is pretty subtle, and heard mostly as a reduction in resonance.

**Bass** - (not working yet). This is going to be like the "Q Level Compensation" control in the Rossum Evolution filter (and many others).

**Poles** - (not working yet). This si going to be like the "Genus" control in the Rossum Evolution filter.

## Suggestions

The "lower order" lowpass settings, like one and two pole can sound nice and bright. Using 1P LP with the Voicing all the way up and a decent drive can get some fairly bright sounds.

Sometimes the Edge control will make the sound darker or brighter. Use it to dial in the sound.

Play a sin wave into the filter and watch the output on a spectrum analyzer. This may help reveal what the different distortion controls do. Similarly, if you run white noise into a the spectrum analyzer will show you the frequency response. Use this to learn more what Type, Caps, Q, and Fc co