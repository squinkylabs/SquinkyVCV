# Release notes for Squinky Labs modules

## 0.6.11

Bug fix. High pass filters added to Shaper in 0.6.10 generate hiss. This release quiets them.

## 0.6.10

Bug fixes:

* Some Chebyshev patches put out a lot of DC voltage.

* Chebyshev harmonics were not as pure as they could be.

* Shaper would sometimes output DC, so we put a 4 pole high pass filter at 20Hz on the output.

Updated the manual for Chebyshev to clarify how to use it as a harmonic VCO and as a dynamic waveshaper.

We also tweaked the output levels of the rectified shapes, as the DC had confused our calibration.

## 0.6.9

Introduced three new modules: EV3, Gray Code, and Shaper.

Added a trim control for external gain CV in Chebyshev. Previously saved patches may require that the gain trim be increased.

Minor graphic tweaks to module panels.

## 0.6.8

Introduced Chebyshev waveshaper VCO.

Introduced release notes.

Lowered the distortion in sin oscillator that is used in all modules.

Re-ordered and re-worded module names in the browser.