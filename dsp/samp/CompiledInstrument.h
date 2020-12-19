#pragma once

#include <memory>
#include <string>
#include <vector>
#include <map>

class SKeyValuePair;
class SInstrument;
class SRegion;
using SKeyValuePairPtr = std::shared_ptr<SKeyValuePair>; 
using SKeyValueList = std::vector<SKeyValuePairPtr>;
using SInstrumentPtr = std::shared_ptr<SInstrument>;
using SRegionPtr = std::shared_ptr<SRegion>;

namespace ci
{

/** left hand side of an SFZ opcode.
 */
enum class Opcode {
    NONE,
    HI_KEY,
    LO_KEY,
    AMPEG_RELEASE,
    LOOP_CONTINUOUS,
    LOOP_MODE,
    LOOP_START,
    LOOP_END,
    PITCH_KEYCENTER,
    SAMPLE
};

enum class DiscreteValue {
    LOOP_CONTINUOUS,
    NO_LOOP,
    NONE
};

/**
 * This holds the right had side of an opcode
 */
class Value {
public:
    int numeric;
    DiscreteValue nonNUmeric;
    std::string string;
};

using ValuePtr = std::shared_ptr<Value>;

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

class VoicePlayInfo {
public:
    bool valid = false;
    int sampleIndex = 0;
    bool needsTranspose = false;
    float transposeAmt = 1;

    bool canPlay() const {
        return valid && (sampleIndex > 0);
    }
};
using VoicePlayInfoPtr = std::shared_ptr<VoicePlayInfo>;
using CompiledInstrumentPtr = std::shared_ptr<class CompiledInstrument>;

class CompiledInstrument {
public:
    static CompiledInstrumentPtr make(const SInstrumentPtr);
    void getInfo(VoicePlayInfo&, int midiPitch, int midiVelocity);
    void _setTestMode() {
        testMode = true;
    }

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

    /** Returns wave index
     */
    int addSampleFile(const std::string& s);
};



using KeysAndValuesPtr = std::shared_ptr<KeysAndValues>;
KeysAndValuesPtr compile(const SKeyValueList&);
//CompiledInstrumentPtr compile(const SInstrumentPtr);

/**
 * finds all the key/value pairs and expands them in place.
 */
void expandAllKV(SInstrumentPtr);



}