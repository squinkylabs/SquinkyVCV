# Octal Slew

This module has 8 independent lag generators, and 8 VCAs. It may be used to smooth out gate signals so that they don't cause pops, or it may be used as a VCA to gate an audio signal with a simple envelope generator.

Each of the 8 channels is independent, although they all share common rise and fall settings.

To use a channel as a lag unit, patch the CV to be lagged into the gate input (first column). A lagged version will be available at the output (last column).

To use a channel as a VCA/AR, patch a 0..10 v CV into the gate input (first column). Patch the audio to be gated into the audio input (second column). The output will have the audio input multiplied by the output of the lag generator, which in this case should be thought of as an AR (attack/release) envelope generator.
