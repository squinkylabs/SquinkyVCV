# Thread Booster

**Updates in Progress - these instructions not correct any more**

Check back later in master branch.

We have two plugins, “Thread Booster” and “CPU hog”. These are experiments to determine if boosting the priority of the audio thread in VCV Rack will help mitigate the pops and clicks that users frequently report. We have found that this works in concocted test cases on Windows – more to be done.

Thread Booster has a UI that lets you boost the priority of the audio thread. There are three arbitrary settings: normal, boosted, and real time. When the switch is in the bottom position, the plugin does nothing; the audio thread keeps its default priority. In the boost (middle) position, it sets the thread priority to the highest priority non-real-time setting. In the real-time position it attempts to set if to the middle priority in the real-time policy.

If setting the priority fails, the red error light lights up, and the priority stays where it was last.

This is all quite platform specific; here’s what we have observed so far:

* In Windows, the boost setting works, and does not require admin rights. The real time setting always fails, even when running as admin.
* On mac, both settings work, and neither requires running as root.
* On Linux (Ubuntu 16) there is only a single non-real-time priority, so the boosted setting fails. The real-time setting does work if you sudo rack.

If you try this plugin, be aware that changing Rack’s thread priority behind its back may case some things to misbehave. We have not seen this, but it’s a real possibility. Running in the real-time policy could lock your computer up, although this seems unlikely since there will only be one audio thread running at this elevated policy.
For those willing to test Thread Booster we ask:

* With a fully loaded session that starts to make pops and clicks, does thread booster fix it?
* If you are able to run realtime, is there any noticeable difference between boosted and real-time?
* After examining the code in ThreadBoost.h, do you have any suggestions for better ways to set the thread priority?

There is also a plugin called “CPU Hog”, which does what it says. This is our concocted test case. This plugin can:

* Spin up worker thread, each of which will saturate a single CPU core at default priority.
* Simulate a very slow graphics drawing call by sleeping in the draw function.

The only “UI” is a display of numbers called “SleepStep”. This is shows the number of time an audio step() call occurred while the plugin was already in the draw() call. We wanted to verify that long drawing time may be interrupted by audio.

Since it has no UI to speak of, you need to edit the source code to do anything. At the top of CPU_Hog.cpp, there are two variables:

```c++
static const int numLoadThreads=1;
static const int drawMillisecondSleep=0;
```

Here is what we did in our test:

* Created a simple patch of a low frequency triangle wave going to the audio interface.
* Added one instance of CPU_Hog set to hog one CPU.
* Kept increasing the number of threads until pops were plainly audible.
* Added an instance of Thread Boost to see if we could make the pops go away.

On Windows, we found pop started to happen when we used three cpus. Or, if the draw time was really long, it would only take two cpus of hogging.

Using Thread Booster in the “boost” setting fixed it.

As we said, this test case is artificial. In the real world are pops and clicks ever caused by other threads (outside of VCV) running and competing for CPU cores? We don’t know. But we suspect that running the audio thread at a higher priority will lessen them.
Btw: rather than continuing to add one to numLoadThreads and re-build, we imagine that just running additional instances, all set to 1, might do the same thing and be easier.

If you try this, and want to report any results feel free to use our GitHub issues. Or a comment on our Facebook page if you prefer.