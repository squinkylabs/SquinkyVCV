# Filter

The filter module is a work in progress. The documentation is for the parts that are working.

Filter is yet another ladder filter, but one that combines many of the ladder enhancements that have been discovered over the decades since the Moog filter first came out.

Filter Provides many more shapes that just four pole low-pass. We believe this innovation was first used in the Oberheim Matrix-12, and it is that implementation we have borrowed.

Like the Moog filter, Filter has a pleasant mild distoration to make it sound fat.

## Controls

**Fc** - filter cutoff.

**Resonance** - filter resonance.

**Type** - filter response type. Choices are four pole lowpass, three pole lowpass, two pole lowpass, one pole lowpass, two pole bandpass, two pole highpass with one pole lowpass, three pole highpass with one pole lowpass, four pose bandpass, one pole lowpass with notch, and three pole allpass with one pole lowpass.

Note that many of these filter type are not what you would normally expect from, say a highpass filter. But they are all useful sounds. Also note that technically when the resonance it turned up, they are all going to have four poles - of course it is impossible to truly make a one-pole resonant filter.

**Drive** - controls the signal level going into the filter. More drive gives more distortion.