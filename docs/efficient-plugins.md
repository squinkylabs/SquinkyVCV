# Efficient Plugins

## The challenge

When most computer software is slow, the user needs to wait longer for the software to respond. But with real time audio systems like VCV Rack, slowness manifests differently. Typically everything is fine until enough things occur to slow down the audio calculations and the buffers underflow. The results are usually a sudden onset of pops, clicks, or missing audio.

When writing plugins for a real-time audio system like VCV Rack (or VST, Audio Unit, etc.) there is the additional complexity that the “system” is a collections of software from different authors with differing strengths and weaknesses.

One plugin that is a CPU hog, or otherwise misbehaves, can make the system click and pop. For the user it is very difficult to tell which plugin is the culprit. For this reasons, it’s very important that any plugins distributed to end users be well-behaved, and that they document any known issues.

## VCV Rack 1.0

Rack 1.0 will be multi-threaded, which means that most users will see a 3X increase in available CPU. That will allow many of today’s inefficient plugins to work without immediately causing pops and clicks. But that doesn’t mean we will be able to get away with inefficient plugin code. VCV users want to use that extra CPU power to make bigger patches that are more reliable. They don’t want us devs to use it ourselves to run our inefficient code.

## What causes buffer underflows

At the hardware level, most audio interfaces operate on buffers of audio data. The audio software does its processing, and delivers a buffer of audio to the interface. If the last buffer is still playing when the new one is delivered underflow will be avoided. If for some reason VCV Rack is not able to deliver a buffer of audio before the previous one has played, an underflow occurs, and the audio interface can’t play anymore.
Buffer size is settable by the user, and typically ranges from 64 samples to 512.
VCV 0.6 has a single-threaded audio engine. If there aren’t a lot of other programs running and competing for cores, that means VCV will have an entire CPU core to run audio processing.
There are three things that plugin software can do that will cause VCV to underflow:

* If the total CPU processing time of all the plugins plus the overhead of VCV’s audio engine take too long to process the buffer of data, then an underflow will occur.

* If one or more of the plugins is usually pretty fast, but every now and then “spikes” and uses a lot of CPU, then that may cause an underflow.

* If a plugin’s step function directly or indirectly tries to lock a semaphore, it may get locked out. Potentially for a very long time.

## Benchmark your code

Many programmers have found that it is almost impossible to develop high performing code unless there is a way to measure it. On a very basic level it is difficult to know if your code is “good enough” without measuring it. Also, it is a well known truism that most programmers’ guesses about where the time is spent in their code are wrong. The thing that looks like it might be slow turns out to be fast, and the real culprit is hiding somewhere else.

The CPU meters in VCV Rack are a big help, but they are not enough. Programmers must be able to take a measurement, change something, and be able to measure a small (10%) improvement. This enables a quick and reliable feedback loop of measure / think / code / measure.

Luckily audio processing code tends to be small, so as long as you can measure it you can usually find out where the hot spots are by removing some code and measuring again.

Some developers set a goal for CPU usage ahead of time. For example, your audio processing code should take one percent of the processing of one CPU core. If all plugins use only 1%, then a user will be able to use 100 plugins at a time without worrying about pops and clicks. This is of course a gross simplification.

Very simple plugins will take much less CPU than 1%. Some very complex plugins can’t be done with only 1% of a core. That’s fine.

## General efficiency

On modern Intel CPUs, floating point math is very fast. In fact everything is very fast. But VCV only has 20 microseconds or so to run all the plugins, which means if you want to stay at that 1% target, you only have two tenths of a microsecond.

So you must keep the code short, and avoid constructs that are slow. What’s slow? It’s hard to say, that’s why measuring yourself is so important.

It used to be that division was slow, and all the transcendental functions (sine, exp, etc) were very, very, slow.

That’s not always the case now. We’ve seen division being only twice as slow as multiplication. Std::pow(2,x) is pretty fast – we measure it taking only 2% of our 1% target, so you could call it 50 times without blowing your budget.

But we have seen many plugins that spend the bulk of their time in std::sin(). Often just replacing a call to sin() with a faster approximation will have a huge impact on the plugin.

A big factor not directly under the plugin’s control is cache pollution. If all the data a plugin uses is in cache memory it may be accessed very quickly. If it’s all the way in off-chip DRAM it will be much slower to access.

Since a plugin only gets called to render a single sample, and then all the other plugins run, it’s difficult to know or control what’s in cache. Nonetheless using less memory and keeping thing all in contiguous memory can make it faster.

It’s always a good idea to avoid conditional branching if possible, as the CPU’s branch prediction unit can make a bad choice, which can incur a large penalty. This can be critical if a plugin has a lot of conditional branching in an inner loop this is running many times per step call (like an oversampling VCO or distortion).

## Spikes

The ideal for the step function is that it takes the same amount of time to run every time. If sometimes it takes much longer that can be bad. If several plugins spiked at once the system would underflow.

Small and frequent spikes will not cause a problem, because they will be averaged over a buffer and won’t make any buffer take much longer than the others. But if the spike occurs less frequently but is large, it will cause a problem.

One common cause of spikes in a step function is a plugin that avoids expensive computations by only updating things when an input or parameter changes. For example, many filters are very efficient at processing data, but take much, much longer to update their internal coefficients when a knob or control voltage changes. So it is a common optimization to only do the calculations when needed. That can be a workable optimization, but make sure to test the worst case performance.

Another cause of spikes is block processing. These kinds of plugins save up input in a buffer until they have a preferred amount of data, then process the entire block on one step() call. An example is an FFT analyzer that stores up samples until it has a whole “frame”, at which time it analyzes the entire block. Note that this would be a poor way to implement an FFT analyzer, most probably do not do this.

If you know your plugin is spikey, consider whether that’s going to be “OK”. Try to benchmark the worst case (typically the worst case is when the audio buffer size is small). Often it is necessary to change the internals of a plugin to make it possible to get a stable reading of the worst case

## Calling external code

There are two or three problems when you call code from your step function that you didn’t write. One is that you may not have any idea how fast or slow they are. The other is that they may be fast much of the time, but very slow some of the time (spikey). 

Functions that do I/O, like reading from a disk, reading data from a socket, etc. can take a very long time in some cases. Allocating memory is also often fast, but can again take way too long in some cases. It’s always a bad idea to call these from your module step function.

Calling math/DSP libraries typically won’t have unbounded execution time, like the I/O functions, but they are often inherently slow. Taking an FFT of a million samples of audio is going to take a long time. A filter design package may be very fast or very slow – it may not be obvious until you measure it.

Lastly, unknown code may very will call operating functions that are bad, like malloc() or read().

## Thread blocking

We’ve already touched on why it’s bad to call functions like read(), open(), malloc(), etc., but there is another less obvious reason. These functions must be thread safe – that is, it must be ok to call from different threads simultaneously. Almost always this thread safety is implemented with some kind of lock, typically a mutex. So when software calls one of these functions the lock may already be taken, and your code will need to wait.

How long will you need to wait? There is no way to know. And an obscure condition called “priority inversion” can make these functions wait for a very, very long time in some cases.

The best practice it to never try to lock the audio thread with a mutex.

## Numerical approximations

If a plugin is spending a lot of time calling expensive math function like sin, tanh, etc, then a very effective speedup is to approximate the function.

There are many, many ways of approximating functions, but one of the most common is a polynomial function. Low order polynomials (cubes, squares) may be calculated pretty quickly.

Usually it is difficult to find one low-order polynomial that is "close enough" over the entire range needed. So it is very common to use "piecewise polynomial approximations." Here’s a trivial example:  `y = (x < 0) ? x : x*x;`

It can be tricky to find suitable approximations like this, but there is one technique that is trivially easy – and effective: use an interpolating lookup table with the values and slopes evenly spaced. A lookup table with linear interpolation is of course a special case of piecewise polynomial approximation where the polynomial is of order 1 and the domain of the function is cut up into equal width bins. 

A lookup table like this is often much, much faster than a function like `std::sin()`. But the approximation should be tested in a unit test to see if the worst case error is still adequate.

## Only do expensive calculations when necessary

We’ve already considered audio code that only calculates expensive filter coefficients when a parameter changes, rather than doing it every sample.

This can be effective, but only if the parameter rarely/never changes. If it’s a parameter, and not associated with a control voltage input, then it will only change while the knob is being moved (although it will move quite a bit while that is going on).

The plugin author will need to carefully consider the pros and cons of such a choice. Again, reasonable criteria might include:

* What is the worst case (measure what happens if everything changes as once)?

* Does avoiding the calculation result in significant improvements?

Another very easy and effective case is a plugin with multiple outputs. If each output requires a lot of processing, then it can be a good improvement to not do the calculation if an output is not patched.

## Lower sample rates

There is no rule that your plugins need to process at the same sample rate as the rest of VCV. Often plugins oversample (run at a higher internal rate), but to save CPU cycles a plugin may run its internal processing at a lower sample rate.

For example, if you have an LFO with an upper frequency limit of 100hz, it’s clearly overkill to calculate everything at 44100 Hz.

An even more common opportunity is calculating parameter changes at a lower sample rate, but process the audio at the full sample rate.

For example, we’ve talked about a filter that can process samples very quickly, but takes a long time to recalculate its coefficients in response to CV changes. So don’t do this calculation in every step call, do it at 1/16 of the sample rate.

But a plugin that does this must be very careful that no audio artifacts are introduced. It is very easy to introduce aliasing if you process audio at a different sample rate, and it’s very easy to introduce “zipper noise” if you process the controls at a reduced sample rate.

Often the solution is a simple low-pass filter. If you are calculating an LFO at a lower rate, filter the output to get rid of stair-steps. You can even low-pass filter your biquad filter coefficients (although experts argue about in which cases this is guaranteed to be stable).

While a one-pole low-pass doesn’t take much CPU, it is very easy to improve this using SSE or AVX instructions. With SSE it is very easy to run four floating point filters in parallel for a 4X speedup.

So – be very careful to avoid bad sounding artifacts, but also remember that all VST 2 audio plugins compute their coefficients at a lower sample rate and get away with it.

## Disable unused features

If a plugin is a quad LFO, and only one section is actually patched in, then don’t run any calculations for the unconnected copies. It is very easy to ask a VCV Rack input or output if it is connected.

## Examples

Following are some examples from the Squinky Labs GitHub that illustrate how we implemented some of the tricks described above. Note that this is real code, warts and all, so it’s not always easy to understand it from an isolated fragment. And some of it is just plain ugly and less than perfectly coded.

### Measurement framework

It’s not easy to benchmark your plugin. Among the challenges are:
Running your plugin outside of VCV Rack
Taking accurate repeatable measurements.
Fooling your compiler into not optimizing your plugin code away.
This last one is a little non-intuitive, but if you run your code in a test harness, the compiler may see that you are always putting in zero for the input, or aren’t using the output. In that case it may realize the plugin isn’t doing anything, and generate no code for it.
Here is our measurement framework: <https://github.com/squinkylabs/SquinkyVCV/blob/main/test/MeasureTime.h>
You can see it used in this perf suite to test Booty Shifter: <https://github.com/squinkylabs/SquinkyVCV/blob/0fd0143ed8ddfefdd00a1fd346fe0758d7307672/test/perfTest.cpp#L168>
We have a strange coding style that lets us build the same code as a plugin or as a standalone testable class. There’s more info here <https://github.com/squinkylabs/SquinkyVCV/blob/main/docs/unit-test.md>

### Lookup table

The interpolating lookup we use in all our modules is here: <https://github.com/squinkylabs/SquinkyVCV/blob/main/dsp/utils/LookupTable.h>
Here’s a simple unit test that shows how to use the lookup table to do sines: <https://github.com/squinkylabs/SquinkyVCV/blob/main/test/testLookupTable.cpp#L49>

### SSE

Here’s the lag generator from Chebyshev. It uses SSE2 instructions to process four channels in the time it would otherwise take to do a single channel. <https://github.com/squinkylabs/SquinkyVCV/blob/main/dsp/filters/MultiLag.h#L101>

### Don’t process knobs and CV every step call

Here’s a place in Chebyshev where we only update some things every four samples. <https://github.com/squinkylabs/SquinkyVCV/blob/main/composites/CHB.h#L415>

### Process low frequency stuff at a lower sample rate

In LFN we have a multiband graphic equalizer that would use a lot of CPU if run at the full sample rate. So we run at perhaps 1/20 of the sample rate, then user a simple filter to smoothly bring it back up to full rate. <https://github.com/squinkylabs/SquinkyVCV/blob/main/composites/LFN.h#L313>
