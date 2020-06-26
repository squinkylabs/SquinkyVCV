# Substitute

Substitute is a copy of the VCOs from the Moog Subharmonicon. It’s not finished, but it’s pretty close now. It’s implemented from the Fundamental VCO-1, which has been heavily modified to generate the subs. This VCO has very low aliasing. So, unlike the divisions you would get with a typical divider, there is almost no aliasing in these subs.

Oh, except there is a pretty bad bug that while the Sawtooth subs are very clean, the square wave subs have a lot of digital grunge in them. I'd like to get rid of it, but so far have not been able to.

I've found that there are pretty many bugs in this thing ATM. If you can find reliable ways to reproduce them, please tell me.

At this point it might be best to consult the Moog manual or a video to learn about this gizmo.

Like the Moog unit each "voice" has two fundamentals, and each fundamental has two subharmonics.

There is a waveform control that’s the same as the Moog Square, saw, and a mixture.

By their nature, the subharmonics will often be somewhat out of tune with an even tempered scale. It’s probably best to read the Moog manual or watch a demo to learn more about this, and why you might want to use a just intonation quantizer.

The built in quantizer will quantize what comes in the V/Oct input. Like the Moog, the scale of the quantizer is always based on "C". If you want to get other keys you need to offset the CV, then re-tune it with the pitch controls. It could be that we want the ability to just set the root note directly.

Main limitations: Glitches when you sweep the VCO division rate. Only 8 voices of polyphony. Sometime it just starts glitching on its own. I've seen it completely stop making sound.
