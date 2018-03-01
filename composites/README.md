# About composites
This is a somewhat ugly "architecture" that lets the same module "guts" run inside a VCV plugin, and also turn inside our unit test framework (which does not link against VCV Rack).

You may look at the FrequencyShifter composite as an example.

## To make a composite.
Make a new class in the composites folder. This class is your composite class. This class must work in the test environment and the VCV Widget environment.

Templatize the class so that its base class is the template parameter. (Example: FrequencyShifter class)

Put all the Input, Output, Param, Led enums into the composite class, rather than the normal Widget class.

To use this composite in a test, derive concrete class from TestComposite
```c++
using Shifter = FrequencyShifter<TestComposite>;
```
Now you my use this class in a test. The TestComposite base class give your tests access to the inputs and outputs of your "widget"
```c++
    Shifter fs;

    fs.setSampleRate(44100);
    fs.init();
    fs.inputs[Shifter::AUDIO_INPUT].value = 0;
```

