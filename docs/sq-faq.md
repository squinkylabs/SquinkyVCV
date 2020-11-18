# Frequently asked questions on Seq++

## What is the easiest way to enter notes?

Loading a MIDI file is the easiest, but that's sort of cheating. As far as entering notes directly in Seq++, using a MIDI keyboard and Seq++'s step record feature is by far the easiest.

After that is entering notes from the computer keyboard. This take a little study to memorize some editing commands, but can also be very quick, and is probably the most powerful.

Lastly is using the mouse. This is perhaps the most intuitive method, but also the slowest. In its present form, the mouse interface is note really recommended for entering an entire sequence.

## What should I set the clock to?

Set it to X64, and use a suitable high speed clock source. The higher the clock resolution, the more what you hear will correspond to what you see on the screen. Use lower clock settings for special effects.

Also, it can be less confusing if you hook up the reset and run inputs. That is if your clock source generates those signals.

## Why doesn't Seq++ have all the features of Cubase and the like?

Those program have been around a long time, and a lot of people have worked on them. But we are always adding new features. To make sure your favorite feature makes it to the top of the list, the best thing to do is log an issue un Github [here](https://github.com/squinkylabs/SquinkyVCV/issues). If you don't want to do that, post on facebook or in the VCV Community forum, or send us a message on Facebook.

## I'm trying to make certain notes go to certain voices in VCV

This isn't going to work. Seq++ uses a modified "rotate" algorithm to assign notes to VCV output channels. This gives great results when all the voices have the same timbre. But if different voices have different timbres it will be a disaster, since the notes will tend to move arbitrarily between voices.

This is very much like controlling a MIDI synthesizer from a MIDI sequencer. All the voices on a given MIDI channel must all sounds the same, and indeed they always do.

Bottom line - if you want to have two different note sequences that are played on different "instruments", use two instance of Seq++. Just like you would use two tracks in a DAW to do this.

## I can't find all the entries in the context menu

There are two different context menus, and each has different things. One comes up when you right-click on the grey panel. The other one comes up when you right click on the note grid.

## Many of the notes aren't playing

Usually this is because the patch you are trying to drive with Seq++ does not respond correctly to polyphonic input. For example, the output is going to a mixer that is not polyphonic. Or the polyphonic VCO is driving a VCF that isn't polyphonic.

If you are having a problem like this, try driving you patch from a polyphonic keyboard and make sure the patch works correctly.

## Some notes are cutting off other notes

Sometimes the polyphony of the music in Seq++ requires a higher polyphony setting, either in Seq++ or down the line.

## Some notes aren't playing or they are all wonky

Make sure the clock rate is high enough, and the polyphony is high enough. These two settings are by far the most common cause of unhappy playback. If these don't fix it, you have found a bug - please report it.

## Seq++'s run state is the opposite of the clock source

TL;DR: press Seq++'s run button until it matches the run state in the clock generator. Now Seq++'s run state will track the master.

The run state "toggles" each time the button is pressed, and each time the run input CV goes from low to high. So when Seq++ and the master don't agree, pressing run on the master will just make them both toggle, and they still won't agree.

## Is it necessary to use a clock generator module?

No, it isn't. You can any signal source to clock Seq++. But there are advantages:

* It's essential if you are using more than one sequencer.
* Having tempo read out in BPM is easier for most people than "Hertz" or the like.
* Other features of a particular master clock may be useful to you.
