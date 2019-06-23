# Change log for Squinky Labs modules

## 1.0.1

New modular mixer (Mix-4M and Mix-4X).

Changed the context menu display for our instruction manuals to be clearer.

## 1.0.0

Released 1.0 versions of release 0.6.16

## 0.6.16

Fixed bug in Mixer-8 where the X inputs either did not work or crashed Rack.

Smoothed out the resonance of Stairway for high resonance, high frequency sweeps.

Adjusted the response of the resonance control in Stairway to make is respond more naturally.

## 0.6.15

New modules: Stairway, Mixer-8, and Slade.

Direct link to the instruction manual for each module is available from the context menu (right click on the module).

Shaper is now stereo.

Buttons now only active on primary (left) button press.

Increased the LFO outputs from Chopper to five volts.

Fixed a typo on the LFN panel graphic.

## 0.6.14

New Module: Saws (Super Saw VCO emulation).

Reduced CPU usage of Formants, Growler, Colors, and Chopper.

EV3 enhancements:

* mixed waveform output now normalized so it stays within 10-V p-p VCV standard.

* Semitone pitch display is now absolute pitch if VCO has a CV connection.

* Pitch intervals now displayed relative to base VCO, rather than relative to C.

Chebyshev enhancements:

* 10 Lag units added to the harmonic volumes. Rise and fall time controlled from knobs and CVs.

* CV inputs for Odd and Even mix level.

* Attenuverters for Slope, odd, and even CV.

* Semitone pitch control knob.

* Semitone and Octave pitch displays.

Shaper enhancement: added AC/DC selector.

LFN enhancement: added XLFN mode, which is 10 times slower. Accessed via context menu.

Colors: changed white knob to Squinky blue.

## 0.6.13

Restore Chebyshev module that disappeared from 0.6.12.

## 0.6.12

Fix bug in LFN when using more than one instance.

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