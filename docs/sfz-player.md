# SFZ Player

## What does the player does
It is a VCV module that can load an SFZ instrument, and convert CV and Gate into the sounds. You can think of is as a sampler like Kontakt (from NI), only more stripped down. And free.

It is fully polyphonic, uses very little CPU, and sounds very good.

In addition to the basic CV and Gate, the player has velocity and pitch modulation inputs. And a fairly unique LFM input that lets you do linear, through zero FM using the samples as the carrier.

## What are SFZ instruments

SFZ Instruments, for the uninitiated, are sample libraries. They are usually free or inexpensive. There are many, many excellent libraries out there for download. Plenty of great sounding pianos, drum kits, orchestral instruments, and on to crazy things.
Because SFZ files are simple text files, and the SFZ format is well documented, it is also easy to make your own SFZ instruments. All you need are some sample files and a text editor/

## Using the player

[TBD] short sections on what the controls and CV do

You must patch something to the V/Oct input and the Gate input, otherwise you won't get any sound. A typical minimal starting patch would use VCV MIDI-CV, SFZ player and an audio output module. Patch the V/OCT, GATE, and VEL outputs from the MIDI-CV to SFZ Player. Patch the output of SFZ player to the audio output. Select a convenient keyboard from MIDI-CV, and set the polyphony to 4 (or whatever you like).

You must also load an SFZ instrument. See section below on where to get one, if you do not already have any. Bring up the context menu by right clicking on the SFZ Player, and select "Load sample file.". You should see the main display at the top begin to load files. This can be instantaneous to tens of seconds, depending on how large the sample library is. When everything loads correctly the display will show you the name of the SFZ Instrument, and the range of pitches over which it responds.

At this you would be able to play on the keyboard and have it respond like a typical sampler. If something goes wrong, there will be an error message in the main display.

## Where to find SFZ

You will need to download some SFZ instrument to get any sound. There are many our there - here are just a few of them. The following aare popular and work well with SFZ player. All are free.

**Versilian Studios**: https://vis.versilstudios.com/index.html
Many free SFZ and many reasonably prices ones. They have two must have collections:

* VCSL: a collection of all kinds of sampled instruments.
* VSCO-2: a collection of mostly orchestral instruments.

The **K18 upright piano** is very popular:  http://rudifiasco.altervista.org/

The **Ivy Piano** is very high quality. Due to the large size, download it usually via torrent or small donation: https://www.ivyaudio.com/Piano-in-162

The **Salamander drum** kit is also a popular instrument. https://archive.org/details/SalamanderDrumkit.

Just using the free instruments listed above will give you many, many useful and good sounding. But there are an enormous number of them out there if you are willing to do a little web searching.

## Plays a subset of SFZ

The SFZ specification is huge, and really designed for implementing a super high end sampled instrument. For a variety of reasons, this player does not implement the full SFZ specification, but only a subset of it.

The result for the user is that any given SFZ instrument may play perfectly, or may play poorly or not at all. So it’s going to be a matter of trial and error. Often the instrument will play just fine. Sometimes not. Unless you are in incurable tinkerer, there isn’t much you can do if an instrument doesn’t play correctly in our player. Time to move on and try an alternative.

## Links [TBD]
How does it work
More about SFZ compatibility, and what opcode are implemented.
How to make your own SFZ files.
Converting sample files to play better in player
More on obtaining good SFZ files


