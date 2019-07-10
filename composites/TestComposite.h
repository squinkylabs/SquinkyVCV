#pragma once

#include <vector>

struct Light
{
    /** The square of the brightness value */
    float value = 0.0;
    float getBrightness();
    void setBrightness(float brightness)
    {
        value = (brightness > 0.f) ? brightness * brightness : 0.f;
    }
    void setBrightnessSmooth(float brightness)
    {
    }
};

#if 0
struct Port
{
    /** Voltage of the port, zero if not plugged in. Read-only by Module */
    float value = 0.0;

    /** Whether a wire is plugged in */
    bool active = false;
    bool isConnected()
    {
        return active;
    }

    Light plugLights[2];
    /** Returns the value if a wire is plugged in, otherwise returns the given default value */
    float normalize(float normalValue)
    {
        return active ? value : normalValue;
    }
};
#endif

static const int PORT_MAX_CHANNELS = 16;


struct alignas(32) Port
{
/** Voltage of the port. */
    union
    {
 /** Unstable API. Use getVoltage() and setVoltage() instead. */
        float voltages[PORT_MAX_CHANNELS] = {};
        /** DEPRECATED. Unstable API. Use getVoltage() and setVoltage() instead. */
        float value;
    };
    union
    {
 /** Number of polyphonic channels
 Unstable API. Use set/getChannels() instead.
 May be 0 to PORT_MAX_CHANNELS.
 */
        uint8_t channels = 0;
        /** DEPRECATED. Unstable API. Use isConnected() instead. */
        uint8_t active;
    };
    /** For rendering plug lights on cables.
    Green for positive, red for negative, and blue for polyphonic.
    */
    Light plugLights[3];

        /** Returns whether a cable is connected to the Port.
    You can use this for skipping code that generates output voltages.
    */
    bool isConnected()
    {
        return channels > 0;
    }
};

struct Input : Port
{
};
struct Output : Port
{
};


/**
* Base class for composites embeddable in a unit test
* Isolates test from VCV.
*/

class TestComposite
{
public:
    TestComposite() :
        inputs(40),
        outputs(40),
        params(60),
        lights(20)
    {

    }
    virtual ~TestComposite()
    {
    }
    struct Param
    {
        float value = 0.0;
    };

 

#if 0   // old ones
    struct Input
    {
        /** Voltage of the port, zero if not plugged in. Read-only by Module */
        float value = 0.0;
        /** Whether a wire is plugged in */
        bool active = false;
        Light plugLights[2];
        /** Returns the value if a wire is plugged in, otherwise returns the given default value */
        float normalize(float normalValue)
        {
            return active ? value : normalValue;
        }
    };

    struct Output
    {
        /** Voltage of the port. Write-only by Module */
        float value = 0.0;
        /** Whether a wire is plugged in */
        bool active = true;
        Light plugLights[2];
    };
#endif

    std::vector<Input> inputs;
    std::vector<Output> outputs;
    std::vector<Param> params;
    std::vector<Light> lights;

    float engineGetSampleTime()
    {
        return 1.0f / 44100.0f;
    }

    float engineGetSampleRate()
    {
        return 44100.f;
    }

    virtual void step()
    {
    }
};
