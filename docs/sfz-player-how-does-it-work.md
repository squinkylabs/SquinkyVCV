# How does SFZ Player work

Each "voice" of SFZ Player is quite simple. There is a monophonic sample streamer that uses a cubic spline for high quality interpolation. The pitch of the sample streamer it determined by the V/Oct CV, as well the the exponential CV input and knobs.

There is also a through zero linear FM input that will phase modulate the sample streamer.

After the sample stream is a VCO driven by an ADSR envelope generator. The ADSR settings are all set from the SFZ file value.

When a gate goes from low to high, logic in the player looks at the pitch and velocity inputs, and looks at the entire SFZ file, and decides what sample to play. It sets up a sample streamer at the correct pitch playing the correct sample. It re-triggers the ADSR generator. Then, as long as a gate stays high it keeps running the streamer, ADSR, VCA, and FM processing.

To load a new SFZ instrument a lot of computation is required, a lot of file loading, and a lot of memory allocation. These things would most surely cause click and pops and interfere with VCV if done on an audio engine thread or on the UI thread. So SFZ Player does all of this on a background thread running at default priority. This lets SFZ Player load files quickly with no danger to VCV's audio.

Because the sample playback is fairly simple, and the file handling is all done on a background thread, SFZ Player integrates well with other VCV modules. It is not a CPU hog, and like most VCV modules it only introduces a one sample delay.
