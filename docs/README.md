# Squinky Labs modules for VCV Rack

All of our plugins are free and open source. The [instruction manual](booty-shifter.md) describes all of the released modules.

The [release notes](release-notes.md) describe recent changes.

All of our released modules may be found in the [VCV Rack plugin manager] (https://vcvrack.com/plugins.html). This is by far the easiest way for most users to install our modules and keep them up to date.

It is also quite easy to clone this repo and build them yourself. In order to do this, however, you must first download and build [VCV Rack itself](https://github.com/VCVRack/Rack).

## More information for developers

We wrote up some tips on how to write plugins that don't use too much CPU. That infomtion is in [Writing Efficient Plugins](efficient-plugins.md).

Documented our experiences optimizing VCO1. They are in [Notes about the creation of Functional VCO-1](vco-optimization.md).

Many of our plugins go to great lengths to avoid aliasing distortion: [Some information about aliasing](aliasing.md).

## Building source

As with all third-party modules for VCV, you must:

* Clone the VCV Rack repo.
* Build Rack from source.
* Clone SquinkyVCV in Rack’s plugins folder.
* `CD SquinkyVCV`
* `make`

Our `Makefile` is currently set up to build for the forthcoming VCV Rack 1.0. If you would like to build for Rack 0.6.x, then edit Makefile here:

```make
# compile for V1 vs 0.6
FLAGS += -D __V1
```

Remove the __V1 flag and it will build for 0.6.x.

## Unit testing framework

We have reasonably thorough tests for our code. Some of this might be of interest - it's [here](unit-test.md).

That document also describes how to run all the tests, including the performance test suite.