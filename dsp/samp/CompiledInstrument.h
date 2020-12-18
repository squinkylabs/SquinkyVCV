#pragma once

#include <memory>
#include <vector>
#include <map>

class SKeyValuePair;
class SInstrument;
using SKeyValuePairPtr = std::shared_ptr<SKeyValuePair>; 
using SKeyValueList = std::vector<SKeyValuePairPtr>;
using SInstrumentPtr = std::shared_ptr<SInstrument>;

namespace ci
{

/** left hand side of an SFZ opcode.
 */
enum class Opcode {
    NONE,
    HI_KEY,
    LO_KEY
};

enum class DiscreteValue {
    NONE
};

/**
 * This holds the right had side of an opcode
 */
class Value {
public:
    int numeric;
    DiscreteValue nonNUmeric;
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
};

class CompiledInstrument {
public:
    void getInfo(VoicePlayInfo&, int midiPitch, int midiVelocity);
    void _setTestMode() {
        testMode = true;
    }
private:

    bool testMode = false;
};
using CompiledInstrumentPtr = std::shared_ptr<CompiledInstrument>;


using KeysAndValuesPtr = std::shared_ptr<KeysAndValues>;
KeysAndValuesPtr compile(const SKeyValueList&);
CompiledInstrumentPtr compile(const SInstrumentPtr);

/**
 * finds all the key/value pairs and expands them in place.
 */
void expandAllKV(SInstrumentPtr);



}