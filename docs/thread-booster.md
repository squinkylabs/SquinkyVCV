# Thread Booster

Thread booster is an experiment to determine if boosting the priority of VCV Rack's audio rendering thread will decrease the annoying pops, ticks, and dropouts that many users are experiencing.

While we have seen this work in some concocted test cases, we do not currently know if this will help in the real world. If you try this, please report back to us on our GitHub issues page.

Thread Booster has a UI that lets you boost the priority of the audio thread. There are three arbitrary settings: normal, boosted, and real time. When the switch is in the bottom position, the plugin does nothing; the audio thread keeps its default priority. In the boost (middle) position, it sets the thread priority to the highest priority non-real-time setting. In the real-time position it attempts to set it to the middle priority in the real-time policy.

If setting the priority fails, the red error light lights up, and the priority stays where it was last.

If you are not up on your operating system thread scheduling, just think of it as "do nothing", "boost a little", and "boost a whole bunch".

## Installing Thread Booster

We have binaries for Mac and Windows posted on our [releases page](https://github.com/squinkylabs/SquinkyVCV/releases).

These release must be unizipped and copied to VCV plugins folder. More info [here](installing-binaries.md).

## Building Thread Booster

Of course you can build the plugins from source for all platforms. But by default the experimental plugins are not build. see [experimental plugins](experimental.md)

## Caveats and limitations

This is all quite platform specific; here’s what we have observed so far:

* In Windows, the boost setting works, and does not require admin rights. The real time setting always fails, even when running as admin.
* On mac, both settings work, and neither requires running as root.
* On Linux (Ubuntu 16) there is only a single non-real-time priority, so the boosted setting fails. The real-time setting does work if you sudo rack.

If you try this plugin, be aware that changing Rack’s thread priority behind its back may cause some things to misbehave. We have not seen this happen, but it’s a real possibility. Running in the real-time policy could lock your computer up, although this seems unlikely since there will only be one audio thread running at this elevated policy. Running at any elevated priority could make something in VCV or the plugins misbehave or glitch.

Note that Thread Booster has no effect on the main VCV Rack process, or its UI thread - it only affects the audio thread.

## Questions for testers

Please use our [GitHub issues](https://github.com/squinkylabs/SquinkyVCV/issues) to send us feedback. Some things we are curious about:

* With a fully loaded session that starts to make pops and clicks, does thread booster fix it?
* If you are able to run realtime, is there any noticeable difference between boosted and real-time?
* For all reports, please list operating system and version (i.e. Windows-10, OSX 10.12, Ubuntu 16.04).
* Programmers: after examining the code in ThreadBoost.h, do you have any suggestions for better ways to set the thread priority?

## CPU hog

There is also a plugin called “CPU Hog”, which does what it says. This is our concocted test case. This plugin can:

* Spin up worker threads, each of which will saturate a single CPU core at default priority.
* Simulate a very slow graphics drawing call by sleeping in the draw function.

We do not provide binaries for this plugin, as it is only of use to hackers, who may easily build it. To build it, pass `_CPU_HOG=1` on the command line to `make`. 

The only “UI” is a display of numbers called “SleepStep”. This is shows the number of time an audio step() call occurred while the plugin was already in the draw() call. We wanted to verify that long drawing time may be interrupted by audio.

Since it has no UI to speak of, you need to do one of two possible actions to use the CPU hog:

* You may load down one CPU core for each instance you  inserts.
* You may change the source code to spin up more threads.

 At the top of CPU_Hog.cpp, there are two variables:

```c++
static const int numLoadThreads=1;
static const int drawMillisecondSleep=0;
```

Here is what we did in our test:

* Created a simple patch of a low frequency triangle wave going to the audio interface.
* Added one instance of CPU_Hog set to hog one CPU.
* Kept increasing the number of threads until pops were plainly audible.
* Added an instance of Thread Boost to see if we could make the pops go away.

On our four core Windows box, we found pops started to happen when we used three CPUs. Or, if the draw time was really long, it would only take two CPUs of hogging.

Using Thread Booster in the "boost" setting fixed it.

As we said, this test case is artificial. In the real world are pops and clicks ever caused by other threads (outside of VCV) running and competing for CPU cores? We don’t know. But we suspect that running the audio thread at a higher priority will lessen them.

## Known issues

On the Mac, Boosting and the switching back to normal will not restore the original priority. It will actually be running at a lower priority.

When the Thread Booster comes up initially, no LEDs are illuminated - normal should be.