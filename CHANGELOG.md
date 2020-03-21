# Change log for Squinky Labs modules

## 1.0.7

Seq++ now supports the "portable sequence format", so you can cut and paste notes between Seq++ and other sequencers that support it.

Seq++ added the "Hookup Clock" command to patch and configure clocks from Impromptu Modular.

Seq++ enhancement: The "auto" versions of the make triad command work a little better, and are more likely to come up with decent voice leading.

Seq++ bug fix: Make triads now uses the normal voicings for chord inversions [[GitHub issue]](https://github.com/squinkylabs/SquinkyVCV/issues/132)

## 1.0.8

Saws is now polyphonic.

## 1.0.7

"Hookup Clock" feature to patch compatible clocks into Seq++.

## 1.0.6

Seq++ new feature - "xforms".

Enhanced FAQ for Seq++.

Seq++ bug fix: Subrange loop now saved with patch. [[GitHub issue]](https://github.com/squinkylabs/SquinkyVCV/issues/127)

Seq++ bug fix: Tab key frequently doesn't work in note grid. [[GitHub issue]](https://github.com/squinkylabs/SquinkyVCV/issues/125)

Seq++ bug fix: Midi File I/O shifts pitch by an octave. [[GitHub issue]](https://github.com/squinkylabs/SquinkyVCV/issues/123)

Mixer bug fix: Typo in manual. [[GitHub issue]](https://github.com/squinkylabs/SquinkyVCV/issues/121)

EV3 bug fix: Tooltip for waveforms comes up in wrong place. [[GitHub issue]](https://github.com/squinkylabs/SquinkyVCV/issues/120)

Seq++ bug fix: Can't reliably change snap to grid settings from context menu. [[GitHub issue]](https://github.com/squinkylabs/SquinkyVCV/issues/119)

## 1.0.5

Seq++ has MIDI file input.

Seq++ has a step recorder.

Inputs on ExFor and Form mixers are now polyphonic. [[GitHub issue]](https://github.com/squinkylabs/SquinkyVCV/issues/90).

Modular mixers now support multi-solo by control-clicking on the solo buttons. [[GitHub issue]](https://github.com/squinkylabs/SquinkyVCV/issues/88).

Saws now has stereo outputs.

Seq++ bugs fixed:

* Articulation setting of 100% was acting like 85%.
* Notes were getting dropped when played with coarse clock.
* Dragging durations with the mouse were making zero length notes.
* Notes that start in the previous two bars are now drawn. [[GitHub issue]](https://github.com/squinkylabs/SquinkyVCV/issues/112)
* Notes re-triggering seemingly randomly when there are plenty of voices. [[GitHub issue]](https://github.com/squinkylabs/SquinkyVCV/issues/98)
* Inserting notes wasn't scrolling the the next bar when it should have. [[GitHub issue]](https://github.com/squinkylabs/SquinkyVCV/issues/113)
* Sometimes the purple track end marker wasn't drawing (depending on grid settings).
* Stuck notes when stopping seq in the middle and then editing. [[GitHub issue]](https://github.com/squinkylabs/SquinkyVCV/issues/117)

Fixed bugs in Polygate that were causing stuck notes when CV inputs changed with gate high. [[GitHub issue]](https://github.com/squinkylabs/SquinkyVCV/issues/105)

Seq++ image in the module browser new looks correct, rather than a black blob. [[GitHub issue]](https://github.com/squinkylabs/SquinkyVCV/issues/99)


## 1.0.4

Three new modules: Seq++, Gateseq, and ExTwo.

Improved the drawing speed of Saws, EV-3 and Chebyshev. Now use 4X less CPU to draw.

Made the "choice widget" used in Stairway only response on left mouse down, so it will come up accidentally less often.

Changed all buttons so they respond to command-click on the Mac, rather than control-click.

Slight cleanup of Gray Code panel.

## 1.0.3

EV-3 gained a lot of aliasing distortion in the 1.0.0 release. Now it's fixed.

## 1.0.2

Fixed bug in Form and ExFor that made solo and mute activate occasionally when dragging volume knob.

Deprecated Functional VCO-1.

Fixed some documentation errors.

## 1.0.1

New modular mixer (Mix-4M and Mix-4X).

Changed the context menu display for our instruction manuals to be clearer.

Fixed mistakes and omissions in the Stairway manual.

## 1.0.0

Released 1.0 versions of release 0.6.16.

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