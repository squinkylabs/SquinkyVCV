# Squinky Labs modules for VCV Rack

All of our released modules may be found in the [VCV Rack plugin manager] (https://vcvrack.com/plugins.html)

# Information for developer and experimenters

There are various test modules, test code, and other good things hidden away in this repo. We will try to point you to some that may be of interest.

In general, many of the folders here have a README with more information.

## Building source

As with all third-party modules for VCV, you must:

* Clone the VCV Rack repo.
* Build Rack from source.
* Clone SquinkyVCV in Rack’s plugins folder.
* `CD SquinkyVCV`
* `make`

## Experimental modules

At any given time, there may partially finished "experimental" modules in this repo. To build all the modules, including the experimental ones, simply run `make _EXP=1`.

If there are any experimental modules, they will tend to have very ugly panels, may not work very well etc... That said if you build and test these, please feel free to log a GitHub issue. Any suggestions for making our modules better is always appreciated.

Most of the time there will be an up to date list of the experimental modules [here](experimental.md).

## Unit testing framework

We have reasonably thorough tests for our code. Some of this might be of interest - it's [here](unit-test.md).