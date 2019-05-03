# Filter

The filter module is a work in progress. The documentation is for the parts that are working. Since there is no CV yet you can't really do anything with it.

Filter is yet another ladder filter, but one that combines many of the ladder enhancements that have been discovered over the decades since the Moog filter first came out.

Filter Provides many more shapes that just four pole low-pass. We believe this innovation was first used in the Oberheim Matrix-12, and it is that implementation we have borrowed.

Like the Moog filter, Filter has a pleasant mild distortion to make it sound fat, although it is possible to dial in more and different distortions.

## Important notes

Although the filter can produce many responses besides just four pole lowpass, it it still at its heart a four pole lowpass with some fancy stuff on top. Because of this, many of the responses other than lowpass will work less well the more the filter itself is pushed away from perfection. So with enough *drive*, *edge*, *Caps* and such it will be less and less like a highpass or a bandpass.

## Controls

**Fc** - filter cutoff.

**Resonance** - filter resonance. At the moment it won't usually go into self oscillation. I should fix that.

**Type** - filter response type. Choices are four pole lowpass, three pole lowpass, two pole lowpass, one pole lowpass, two pole bandpass, two pole highpass with one pole lowpass, three pole highpass with one pole lowpass, four pose bandpass, one pole lowpass with notch, and three pole allpass with one pole lowpass.

Note that many of these filter type are not what you would normally expect from, say a highpass filter. But they are all useful sounds. Also note that technically when the resonance it turned up, they are all going to have four poles - of course it is impossible to truly make a one-pole resonant filter.

**Drive** - controls the signal level going into the filter. More drive gives more distortion. Too much and the filter stops working right and gets all flabby.

**Voicing** selects different types of distortion. The first selection is the standard one that is pretty close to a Moog. The "Asym" ones have a lot of even harmonics, the others are all odd harmonics.

**Edge** - 4P Ladder filters are pretty mellow, even when driven hard, because much of the internal distortion get filtered very heavily. This control changes the internal gain structure of the filter so that it still has the same frequency response, but the spectrum will change. Set in the middle is "normal".

**Caps** - simulates using inaccurate capacitor values. At the minimum setting, the are all perfectly matched. As the value is increased, they go to typical values that would be found in a Moog, up to very imprecise values. That said, the effect is pretty subtle, and heard mostly as a reduction in resonance.

**Bass** - (not working yet). This is going to be like the "Q Level Compensation" control in the Rossum Evolution filter (and many others).

**Poles** - (not working yet). This si going to be like the "Genus" control in the Rossum Evolution filter.

## Suggestions

The "lower order" lowpass settings, like one and two pole can sound nice and bright. Using 1P LP with the Voicing all the way up and a decent drive can get some fairly bright sounds.