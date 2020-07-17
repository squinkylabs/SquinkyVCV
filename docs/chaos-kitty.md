# Chaos Kitty

Chaos kitty is a chaos oscillator. Most chaos modules in VCV are for generating CV, but this one is for generating audio. There are three different modes: two different chaos oscillators, and a choice of having a resonant filter on the output.

It’s currently monophonic, although it could easily be poly (I just don’t know why you would want that).
The most “sophisticated” sound is from the “pitched” mode. In that mode all the controls do something. You can get almost pitched sounds. It will track a CV input, and it gets more pitched they higher the resonance is.

Main limitations: it’s really unfinished. Needs a lot of work.

Many of the controls don't do anything at all in most modes.

There are no labels on the knobs. Use tooltips to find out which is which (or now).

## Controls

**Chaos**: In all modes, this control does something. It's often unpredictable. It doesn't smoothly go from a sound that sounds like "less" chaos to "more" chaos, it's more like it sweeps through all kind of chaos. Sometimes a very small change has a huge effect.

**Chaos CV input**: The CV input and companion attenuverter let you control the chaos amount with a CV.

## Controls that only work with "pitched" mode

**Resonance**: Controls the resonant filter that gives "pitched" mode its name. The higher the resonance, the more pitched the result will sound. Less resonance will sound more chaotic.

**Octave**: Shifts the resonant frequency by an octave.

**V/Oct**: Sets the resonant frequency, in combination with the Octave control.

**Brightness**: Does not seem to work right now.

## Two other modes

Both "noise" an "circle" use very simple functions to generate chaos. The hope is that they sound different enough from each other that it makes sense have both of them in here.

Since small changes in the Chaos can make large changes in the sound, it can be tricky to modulate the chaos amount with a CV. For example, even at very low settings running an LFO in here can sound like randomly tuning radio static, rather than a smooth change.

On thing that makes it sound less crazy is to use a sample and hold module to sample an LFO or random source. Clock the S/H from a clock (or anything else). Each time the clock fires you get a very different sound, but in between clocks your ear has time to absorb what's going on.

## Features

There are probably many features that could be added to make Chaos Kitty more useful or more fun. Suggestions welcome.
