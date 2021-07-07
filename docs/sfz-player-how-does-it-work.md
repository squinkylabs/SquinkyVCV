# How does SFZ Player work

When the user loads an SFZ file, SFZ player will turn it into a playable set of regions. It does this by using a conventional three phase, "compiler". In the first phase, lexical analytics, a `lexer` converts the text SFZ file in a list of recognized tokens. During this phase comments are identified and removed, and `#include` files are processed. Next a parser examines the stream of lexical tokens and builds up an abstract syntax tree. This is just a simple data structure that holds all the SFZ headings and opcodes in a convenient format. Lastly, a final pass analyzes the syntax tree and builds up a dictionary of playable regions in a way that makes it very easy and efficient to play back.

This last phase also produces a list of wave/flac files referenced by the SFZ. SFZ player will load all of these into RAM.

Each "voice" of SFZ Player is quite simple. There is a monophonic sample streamer that uses a cubic spline for high quality interpolation. The pitch of the sample streamer is determined by the V/Oct CV, as well as the exponential CV input and knobs. Some SFZ opcodes also affect the pitch. This sample streamer also supports looping, so it may jump around in the wave file image in RAM as it plays.

There is a through zero linear FM input that will phase modulate the sample streamer.

After the sample stream is a VCA driven by an ADSR envelope generator. The ADSR settings are all set from the SFZ file values.

When a gate goes from low to high, logic in the player looks at the pitch and velocity inputs, and looks at the compiled SFZ file, and decides what sample to play. It sets up a sample streamer at the correct pitch playing the correct sample. It re-triggers the ADSR generator. Then, as long as a gate stays high it keeps running the streamer, ADSR, VCA, and FM processing.

In order to give the pitch CV time to percolate though any modules that may be processing pitch, the gate signals are all delayed by 5 samples. This tinny delay may be turned off.

To load and compile a new SFZ instrument a lot of computation is required, a lot of file loading, and a lot of memory allocation. These things would most surely cause clicks and pops and interfere with VCV if done on an audio engine thread or on the UI thread. So SFZ Player does all of this on a background thread running at default priority. This lets SFZ Player load files quickly with no danger to VCV's audio.

Because the sample playback is fairly simple, and the file handling is all done on a background thread, SFZ Player integrates well with other VCV modules. It is not a CPU hog, and like most VCV modules it only introduces a one sample delay.
