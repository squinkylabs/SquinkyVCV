# Filter

The filter module is a work in progress. The documentation is for the parts that are working. Since there is no CV yet you can't really do anything with it.

Filter is yet another ladder filter, but one that combines many of the ladder enhancements that have been discovered over the decades since the Moog filter first came out.

Filter Provides many more shapes that just four pole low-pass. We believe this innovation was first used in the Oberheim Matrix-12, and it is that implementation we have borrowed.

Like the Moog filter, Filter has a pleasant mild distortion to make it sound fat, although it is possible to dial in more and different distortions.

Bug alert - at them moment many of the features only work when set to 4P-LP and "Classic".

## Controls

**Fc** - filter cutoff.

**Resonance** - filter resonance.

**Type** - filter response type. Choices are four pole lowpass, three pole lowpass, two pole lowpass, one pole lowpass, two pole bandpass, two pole highpass with one pole lowpass, three pole highpass with one pole lowpass, four pose bandpass, one pole lowpass with notch, and three pole allpass with one pole lowpass.

Note that many of these filter type are not what you would normally expect from, say a highpass filter. But they are all useful sounds. Also note that technically when the resonance it turned up, they are all going to have four poles - of course it iss impossible to truly make a one-pole resonant filter.

**Drive** - controls the signal level going into the filter. More drive gives more distortion. Too much and the filter stops working right and gets all flabby.

**Voicing** selects different types of distortion. The first selection is the standard one that is pretty close to a Moog. The third one has a lot of even harmonics in it. Need much more work

**Edge** - 4P Ladder filters are pretty mellow, even when driven hard, because much of the internal distortion get filtered very heavily. This control changes the internal gain structure of the filter so that it still has the same frequency response, but the spectrum will change. Set in the middle is "normal".

**Caps** - simulate using inaccurate capacitor values. At the minimum setting, the are all perfectly matched. As the value is increased, they to to typical values that would be found in a Moog, up to very imprecise values. That said, the effect is pretty suble, and heard mostly as a reduction in resonance.

## Suggestions

The "lower order" lowpass settings, like one and two pole can sound nice and brigh. Using 1P LP with the Voicing all the way up and a decent drive can get some fairly bright sounds.