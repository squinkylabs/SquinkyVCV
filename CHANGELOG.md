# Change log for Squinky Labs modules

## 10.0.20

Comp2: removed tooltip from channel select knob.

## 10.0.19

New module: Comp II.

Compressor level detector now has less IM distortion (re-design).

Compressor soft-knee now has smoother shape - sounds better (re-design).

Tooltips for compressor threshold now displayed in dB, for both compressors. [[GitHub issue]](https://github.com/squinkylabs/SquinkyVCV/issues/198)

User may now type into tooltips on Comp (bug fix).

SFZ Player: now understands absolute path to sample files.

## 10.0.18

New module: F2

Changed SFZ default release time to match Sforzando player, not the spec.

## 10.0.17

Fixed links to SFZ Player in the VCV Library, and links to Organ 3

Fixed a crash when right clicking in the Seq++ note area.

Fix crash in Substitute when attempting to set polyphony higher than 8.

## 10.0.16

New module: SFZ Player.

Fixed bugs in Substitute quantizer where just intonation was completely out of tune.

Fixed bug in Comp where newly patched channels didn't have the correct settings until curve changed.

Fixed bug in Comp where gain reduction meters would flicker.

Fixed Seq++ bugs: saving, displaying, and re-loading looped ranges did not work correctly, and could even crash.

## 10.0.15

Re-coded some old modules to make them compatible with the new Library build system.

Fixed bug in Comp - it could only apply 20 dB of compression.

Fixed the broken link to the 4X4 manual.

Small changes to Seq++ manual.

## 1.0.14

New Module: Comp (audio compressor).

## 1.0.13

Bug fix: sometimes the tooltip for Basic VCO waveform disagrees with the actual waveform.

Bug fixes: Both Formants and Growler can go into outer space if you set the Fc all the way up.

Bug fix: the link to the 4X4 manual from the context menu is a dead link.

## 1.0.12

New module: Basic VCO

Bug fixes:

* Accidentally turned off some optimization for version 1.0.11 and 1.0.10. These are now on, making most modules use less CPU.
* The wave-folder modulation index in Kitchen Sink has a bad taper, making it way too sensitive. Now audio taper fixes this and makes it sound better. Old patches with wavefolding will still load correctly.

## 1.0.11

New module: Organ-Three.

Bug Fixes in Ev3:

* It was completely broken in 1.0.10.
* Fixed DC offset in Sawtooth output.
* Fixed DC offset in Even output.
* Fixed enormous DC offset in pulse output.

## 1.0.10

Bug fix: Kitchen Sink plays wrong pitches when polyphony greater than four.

Bug fix: DC offset on sawtooth output from Substitute. Offset increases with frequency, becoming quite large at 1kHz.

Bug fix:  DC offset on pulse wave output from Substitute. Offset increases as the pulses get narrower, becoming quite large at very narrow width.

Documentation fix: Various documentation pages were displaying older versions.

## 1.0.9

New module: Kitchen-Sink
New module: Substitute

## 1.0.8

New sequencer module: 4X4.
Saws is now polyphonic.
Stairway is now polyphonic.

## 1.0.7

Seq++ now supports the "portable sequence format", so you can cut and paste notes between Seq++ and other sequencers that support it.

Seq++ added the "Hookup Clock" command to patch and configure clocks from Impromptu Modular.

Seq++ enhancement: The "auto" versions of the make triad command work a little better, and are more likely to come up with decent voice leading.

Seq++ bug fix: Make triads now uses the normal voicings for chord inversions [[GitHub issue]](https://github.com/squinkylabs/SquinkyVCV/issues/132)

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