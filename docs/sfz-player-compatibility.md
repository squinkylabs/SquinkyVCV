
# SFZ player compatibility

The Sforzando player, from Plogue, is the reference player for SFZ. Almost any SFZ will sound correct when played with that player. As such it can be a valuable tool for evaluating SFZ.

You can get a free download [here](https://www.plogue.com/products/sforzando.html).

## General compatibility

In general you will find that a majority of the free SFZ files will play just fine in SFZ Player. There are some very elaborate, high-end SFZ that won't play so well. The ones from Pianobook, for example. There are also some very simple SFZ that were converted from (often ancient) sound fonts that play poorly in SFZ Player.

As we mentioned earlier, SFZ Player implements a subset of the SFZ specification. So when you load an SFZ, you can expect one of several outcomes:

* It will load and play just fine.
* It will load, but will not play well.
* It will load, but not play at all.
* It will refuse to load.

## In case of problems

If you load an SFZ file and have a problem, you can ignore it and move on, or you can log an issue with us on our [GitHub Page](https://github.com/squinkylabs/SquinkyVCV/issues). It's your choice.

If you have an SFZ that crashes SFZ player, or gives an error message, please report it. If possible zip the SFZ and include it (the SFZ is a pretty small file - it's the samples that are big).

If a file loads but doesn't sound right you can edit the SFZ yourself, if you are that sort. Or you can log an issue with us with a link to download the SFZ. Or you can move on. Your choice.

If you look in the VCV log file, you will probably see entries from SFZ player. These care be quite useful to help understand what is going on. They are typically:

* A list of unimplemented opcode that were ignored when importing the SFZ file.
* A list of regions that were deleted because they were playing at the same time as other regions.
* Sometimes longer versions of error message that are show on the panel.

## Description of capabilities

SFZ Player can only play one sample per voice at a time. So, for example, it can’t play a “damper pedal noise” on top of a piano note. There are no doubt ensemble patches that rely on many instruments playing from one note - they will probably sound very bad. When we load an SFZ that attempts to play multiple samples on the same note, we try to figure out which one is more important, and we play that one. Sometimes we guess wrong.

No continuous controller. Many sample libraries use the “mod wheel” to select/fade alternate samples. For example open and closed hi hat. We don’t implement that.

No built in modulation. An SFZ might have vibrato mapped to a controller. We don’t support any modulation from the SFZ file.

No built in subtractive synth. SFZ spec allows one to define a virtual instrument using classic waveforms, VCF and ADSR settings. These tend to be used heavily in older sound fonts, although they are used in modern SFZ to subtly modify high quality samples.

Many piano samples use “release samples” to accurately record the sound of letting up a key and having the damper fall back onto the strings. We don’t implement release samples, we just ignore them.

SFZ may have several different types of sample files, although the huge majority use wav or flac. We do not read ogg or aiff files, so if you find the rare SFZ instrument that uses them you are out of luck. Unless you are motivated enough to convert them yourself.

"Aria Format" is a superset of SFZ. Aria libraries use a different method of locating their samples which we do not support.

## Future enhancements

Most of the limitations listed can be addressed. Please feel free to contact us and let us know if there is something you would like addressed.

## SFZ Opcodes implemented

* Hikey, lokey, key, pitch_keycenter

* hivel, lovel  (these allow different velocities to play different samples)

* ampeg_release

* amp_veltrack

* default_path

* sample

* seq_length, seq_position, hirand, lorand  (these enable round-robin and random selection of sample. crucial for drums)

* sw_label, sw_last, sw_lokey, sw_hikey, sw_default (these enable "keyswitching", or selecting alternate sounds).

* tune

* volume

* #include

## added in 10.21

* loop_mode
** one_shot mode lets drums that use it play out regardless of gate/trigger length.
** continuous and sustain allow looping of portions of a sample. often used for samples of other synthesizers.

* loop_start, loopstart, loop-end, loopend are used by loop_mode=continuous for specifying loop points within a sample.

* offset, end are often used to "tighten up" poorly edited samples so that the start and end correctly and cleanly.

* oscillator allows samples to act as wavetable oscillators.
