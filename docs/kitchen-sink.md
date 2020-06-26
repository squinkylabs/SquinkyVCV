# Kitchen Sink

This started life a year or more ago as a VCO that had all the feature of a lot of VCOs in one. Then I stopped working on it for a year. Now it’s back but quite different. Now it’s a “tribute” to the FM OP, with a similar control structure, but more features.

This VCO should work pretty well, and shouldn't have a lot of bugs.

There are two main intentions for this VCO. 

* Any place you would use FM-OP you should be able to use this VCO. It should get all sounds FM-OP does, and it should have comparable sound quality.

* The combination of a wavefolder, hard sync, and FM in one VCO means there are a LOT of different sounds inside here.

* The build-in envelope generator is capable of Moog-like "snap"

Note: snap control is on the context menu. Although there are two entries: “snap” and “more snap”, they work as follows: If one is selected (doesn’t matter which one) it will have about the same snap as a minimoog. If both are one it will have more.

In addition to the sine waveform, it has one that can morph from triangle to sawtooth, and a third one that is a wavefolder.

And of course it has through zero phase modulation (most other VCOs say they have linear FM, but they really use PM, because it’s better. Even the DX-7 used PM).

The built-in ADSR is lifted from the Fundamental ADRS, but I made it a little faster and added the Minimoog “snap”. I’m hoping the “snap” will take it a little into “spank territory, but with easier programming since you don’t have to set the “H” time yourself.

Of course it’s fully polyphonic.

The only things that should be unfamiliar from the FM OP is the waveform control (sine, folder, and tri), and the shape param. When set to Tri, shape morphs from tri to saw. With wavefolder it’s the amount of folding. With sin it does nothing. You can get classic wavefolder sounds it you module the shape with an envelope, or a CV.

If you use the sync input, remember that the sync input will be the modulator, and it works best if it is lower in pitch than kitchen sync.

It has octave and ratio. The ration only goes up (x2, x3, x4), since the octave lets you get ½, ¼.

The mod knob controls the pitch modulation depth. Not super useful when using as part of a multi operator FM patch, but useful with the waveshaper.