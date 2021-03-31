# Make your own SFZ Instruments

It is very easy to make you own SFZ Instruments. All you need is a text editor, and some wav files. You may record your own wave file, or you can download from somewhere.

Here we will show some extremely simple examples. To learn more about SFZ, xxxx

As far as text editors, it's possible to use one that is built into your operating system. On Windows notepad works ok, and on Mac TextEdit works ok, as long as you make sure you save as text, and not rtf or some other word processing format.

But - the job is much easier if you use a programmer's text editor. Some common examples are Visual Studio Code, vi, vim, emacs, Sublime, Atom. If you already have a favorite editor, use it. If you don't, download Visual Studio Code. It's 100% free, and there are plugins for SFZ syntax highlighting that make the job slightly easier.

## Samples mapped up the keyboard

This is a very simple example, but realistic. Assume you have 7 wave files, and you want trigger each one from a white key starting at middle C. Further, assume these files are called c.wav, d.wav.... b.wav, and that they are in the same folder as your sfz file you will create.

Because we only want to map each sample to a single key, and because we don't want to transpose, we can use the *key* opcode. So, the first region will look like this:

<region> key=c4 sample=c.wav

That's all you need to do - save that into a file with the extension *.sfz* and it will play. Now, to make this fully realistic, let's set the ADSR release to something long. This will avoid pops, and let your release match the natural release of the sample. So where it the finished SFZ:

```
// This is our first SFZ. Comments like this can be added anywhere

// Putting ampeg_release in a group like this will apply it to all the following regions.
<group> 
ampeg_release=.5
<region> key=c4 sample=c.wav
<region> key=d4 sample=d.wav
<region> key=e4 sample=e.wav
<region> key=f4 sample=f.wav
<region> key=g4 sample=g.wav
<region> key=a4 sample=a.wav
<region> key=b4 sample=b.wav
```
This will play a different sample on each white key in the middle octave. Black keys, and keys outside that range will no play anything.

## One sample stretched over an octave

Now let's make an SFZ that stretches on sample from C4 to C5. We will use a.wav again, and assume that it plays an a4 when played without transposition.
```
<region>
ampeg_release=.5
lokey=c4
hikey=5 
pitch_keycenter=a4
```


