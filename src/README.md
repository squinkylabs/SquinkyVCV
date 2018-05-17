# src folder
This folder contains the top-level Module source code. Almost all of our code that is specific to VCV Rack is in this folder.

Usually all of the code the actually generates and processes audio is elsewhere.
## Experimental Modules
Colors is a colored noise generator. It can generate all the common “colors” of noise, including white, pink, red, blue, and violet. It can also produce all the colors in-between, as it has a continuously variable slope.

Colors has a single control, “slope”. This is the slope of the noise spectrum, from -8 dB/octave to +8 dB/octave.

The slope of the noise is quite accurate in the mid-band, but at the extremes we flatten the slope to keep from boosting  super-low frequencies too much, and to avoid putting out enormous amounts of highs. So the slope is flat below 40hz, and above 8kHz.

### Things to be aware of
When the slope changes, Color needs to do a lot of calculations. While this is normally not a problem, it’s possible that quickly changing the slope of many instances of Colors could cause pops and dropouts.

The slope control does not respond instantly. If you turn the knob, you will hear the change, but if you were to modulate the CV very quickly you might notice the slowness.

