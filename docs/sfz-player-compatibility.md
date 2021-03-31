
# SFZ player compatibility

The Sforzando player, from Plogue, is the reference player for SFZ. Almost any SFZ will sound correct when played with that player. As such it can be a valuable tool for evaluating SFZ.

You can get a free download [here](https://www.plogue.com/products/sforzando.html).

## General compatibility

In general you will find that a majority of the free SFZ files will play just fine. There are some very elaborate, high=end SFZ that won't play so well. The ones from Piano Book, for example.

As we mentioned earlier, SFZ Player implements a subset of the SFZ specification. So when you load an SFZ, you can expect one of several outcomes:

* It will load and play just fine.
* It will load, but will not play well.
* It will load, but not play at all.
* It will refuse to load.

## In case of problems

If you load an SFZ file and have an problem, you can ignore it and move on, or you can log an issue with us on our [GitHub Page](https://github.com/squinkylabs/SquinkyVCV/issues). It's your choice.

If you have an SFZ that crashes SFZ player, or gives an unexpected error message, plese report it. If possible zip the SFZ and include it (the SFZ is a pretty small file - it's the samples that are big).

If a file loads but doesn't sound right you can edit the SFZ yourself, if you are that sort. Or you can log an issue with us with a link to download the SFZ. Or you can move on. Your choice.

## Description of capabilities

It can only play one sample per voice at a time. So, for example, it can’t play a “damper pedal noise” on top of a piano note. There are no doubt ensemble patches that rely on many instruments playing from one note - they will probably sound very bad.

No continuous controller. Many sample libraries use the “mod wheel” to select/fade alternate samples. Like open and closed hi hat. We don’t implement that.

No built in modulation. An SFZ might have vibrato mapped to a controller. We don’t support any modulation from the SFZ file.

Many piano samples use “release samples” to accurately record the sound of letting up a key and having the damper fall back onto the strings. We don’t implement release samples, we just ignore them.

SFZ Player does not implement any form of looping. Because of this many older SFZ that were converted from Sound Fonts will not play correctly.

SFZ may have many different types of sample files, although the huge majority use wav of flac. We do not read ogg or aif files, so if you find the rare SFZ instrument that uses them you are out of luck. Unless you are motivated enough to convert them yourself.

## SFZ Opcodes implemented

* Hikey, lokey, key, pitch_keycenter

* hivel, lovel

* ampeg_release

* amp_veltrack

* default_path

* sample

* seq_length, sep_position, hirand, lorand 

* sw_label, sw_last, sw_lokey, sw_hikey, sw_default

* tune

* volume

* #include
