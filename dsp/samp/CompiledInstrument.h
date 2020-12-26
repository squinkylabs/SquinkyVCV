#pragma once

#include <memory>
#include <string>
#include <vector>
#include <map>

#include "SamplerPlayback.h"

class SKeyValuePair;
class SInstrument;
class SRegion;
class WaveLoader;
class SGroup;

using SKeyValuePairPtr = std::shared_ptr<SKeyValuePair>; 
using SKeyValueList = std::vector<SKeyValuePairPtr>;
using SInstrumentPtr = std::shared_ptr<SInstrument>;
using SRegionPtr = std::shared_ptr<SRegion>;
using WaveLoaderPtr = std::shared_ptr<WaveLoader>;
using SGroupPtr = std::shared_ptr<SGroup>;

namespace ci
{

/** left hand side of an SFZ opcode.
 */
enum class Opcode {
    NONE,
    HI_KEY,
    LO_KEY,
    HI_VEL,
    LO_VEL,
    AMPEG_RELEASE,
    LOOP_CONTINUOUS,
    LOOP_MODE,
    LOOP_START,
    LOOP_END,
    PITCH_KEYCENTER,
    SAMPLE,
    PAN,
    GROUP,  // group is opcode as well at tag
    TRIGGER,
    VOLUME,
    TUNE,
    OFFSET,
    POLYPHONY,
    PITCH_KEYTRACK,
    AMP_VELTRACK
};

enum class DiscreteValue {
    LOOP_CONTINUOUS,
    NO_LOOP,
    ATTACK,         // trigger=attack
    RELEASE,        // trigger= release
    NONE
};

enum class OpcodeType {
    String,
    Int,
    Float,
    Discrete
};

/**
 * This holds the right had side of an opcode.
 * It can hold in integer, a float, a string, or a named discrete value
 */
class Value {
public:
    float numericFloat;
    int numericInt;
    DiscreteValue discrete;
    std::string string;
    OpcodeType type;
};

using ValuePtr = std::shared_ptr<Value>;

/**
 * hold the compiled form of a collection of group attributes.
 */
class KeysAndValues
{
public:
    size_t _size() const {
        return data.size();
    }
    void add(Opcode o, ValuePtr vp) {
        data[o] = vp;
    }

    ValuePtr get(Opcode o) {
        auto it = data.find(o);
        if (it == data.end()) {
            return nullptr;
        }
        return it->second;

    }
private:
    // the int key is really an Opcode
    std::map<Opcode, ValuePtr> data;
};


using CompiledInstrumentPtr = std::shared_ptr<class CompiledInstrument>;


class CompiledInstrument : public ISamplerPlayback {
public:
    static CompiledInstrumentPtr make(const SInstrumentPtr);
    void play(VoicePlayInfo&, int midiPitch, int midiVelocity) override;
    void _setTestMode() {
        testMode = true;
    }

    /**
     * move all the waves from here to wave loader
     */
    void setWaves(WaveLoaderPtr waveLoader, const std::string& rootPath);

private:

    bool testMode = false;
    std::map<int, VoicePlayInfoPtr> pitchMap;

    /**
     * Track all the unique relative paths here
     * key = file path
     * value = index (wave id);
     */
    std::map<std::string, int> relativeFilePaths;    
    int nextIndex = 1;

    void compile(const SInstrumentPtr);
    void compileSub(const SRegionPtr);
    bool shouldIgnoreGroup(SGroupPtr);

    /** Returns wave index
     */
    int addSampleFile(const std::string& s);
};

using KeysAndValuesPtr = std::shared_ptr<KeysAndValues>;
KeysAndValuesPtr compile(const SKeyValueList&);


/**
 * finds all the key/value pairs in a parse tree and expands them in place.
 */
void expandAllKV(SInstrumentPtr);



}