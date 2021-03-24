#include "SamplerSchema.h"

#include <assert.h>

#include <set>

#include "SParse.h"
#include "SamplerErrorContext.h"
#include "SqLog.h"

using Opcode = SamplerSchema::Opcode;
using OpcodeType = SamplerSchema::OpcodeType;
using DiscreteValue = SamplerSchema::DiscreteValue;

// TODO: compare this to the spec
static std::map<Opcode, OpcodeType> keyType = {
    {Opcode::HI_KEY, OpcodeType::Int},
    {Opcode::KEY, OpcodeType::Int},
    {Opcode::LO_KEY, OpcodeType::Int},
    {Opcode::HI_VEL, OpcodeType::Int},
    {Opcode::LO_VEL, OpcodeType::Int},
    {Opcode::SAMPLE, OpcodeType::String},
    {Opcode::AMPEG_RELEASE, OpcodeType::Float},
    {Opcode::LOOP_MODE, OpcodeType::Discrete},
    //  {Opcode::LOOP_CONTINUOUS, OpcodeType::}
    {Opcode::PITCH_KEYCENTER, OpcodeType::Int},
    {Opcode::LOOP_START, OpcodeType::Int},
    {Opcode::LOOP_END, OpcodeType::Int},
    {Opcode::PAN, OpcodeType::Int},
    {Opcode::GROUP, OpcodeType::Int},
    {Opcode::TRIGGER, OpcodeType::Discrete},
    {Opcode::VOLUME, OpcodeType::Float},
    {Opcode::TUNE, OpcodeType::Int},
    {Opcode::OFFSET, OpcodeType::Int},
    {Opcode::POLYPHONY, OpcodeType::Int},
    {Opcode::PITCH_KEYTRACK, OpcodeType::Int},
    {Opcode::AMP_VELTRACK, OpcodeType::Float},
    {Opcode::LO_RAND, OpcodeType::Float},
    {Opcode::HI_RAND, OpcodeType::Float},
    {Opcode::SEQ_LENGTH, OpcodeType::Int},
    {Opcode::SEQ_POSITION, OpcodeType::Int},
    {Opcode::DEFAULT_PATH, OpcodeType::String},
    {Opcode::SW_LABEL, OpcodeType::String},
    {Opcode::SW_LAST, OpcodeType::Int},
    {Opcode::SW_LOKEY, OpcodeType::Int},
    {Opcode::SW_HIKEY, OpcodeType::Int},
    {Opcode::SW_DEFAULT, OpcodeType::Int},
    {Opcode::HICC64_HACK, OpcodeType::Int},
    {Opcode::LOCC64_HACK, OpcodeType::Int}};

static std::map<std::string, Opcode> opcodes = {
    {"hivel", Opcode::HI_VEL},
    {"lovel", Opcode::LO_VEL},
    {"hikey", Opcode::HI_KEY},
    {"lokey", Opcode::LO_KEY},
    {"hirand", Opcode::HI_RAND},
    {"lorand", Opcode::LO_RAND},
    {"pitch_keycenter", Opcode::PITCH_KEYCENTER},
    {"ampeg_release", Opcode::AMPEG_RELEASE},
    {"loop_mode", Opcode::LOOP_MODE},
    //   {"loop_continuous", Opcode::LOOP_CONTINUOUS},
    {"loop_start", Opcode::LOOP_START},
    {"loop_end", Opcode::LOOP_END},
    {"sample", Opcode::SAMPLE},
    {"pan", Opcode::PAN},
    {"group", Opcode::GROUP},
    {"trigger", Opcode::TRIGGER},
    {"volume", Opcode::VOLUME},
    {"tune", Opcode::TUNE},
    {"offset", Opcode::OFFSET},
    {"polyphony", Opcode::POLYPHONY},
    {"pitch_keytrack", Opcode::PITCH_KEYTRACK},
    {"amp_veltrack", Opcode::AMP_VELTRACK},
    {"key", Opcode::KEY},
    {"seq_length", Opcode::SEQ_LENGTH},
    {"seq_position", Opcode::SEQ_POSITION},
    {"default_path", Opcode::DEFAULT_PATH},
    {"sw_label", Opcode::SW_LABEL},
    {"sw_last", Opcode::SW_LAST},
    {"sw_lokey", Opcode::SW_LOKEY},
    {"sw_hikey", Opcode::SW_HIKEY},
    {"sw_default", Opcode::SW_DEFAULT},
    {"hicc64", Opcode::HICC64_HACK},
    {"locc64", Opcode::LOCC64_HACK}};

static std::set<std::string>
    unrecognized;

static std::map<std::string, DiscreteValue> discreteValues = {
    {"loop_continuous", DiscreteValue::LOOP_CONTINUOUS},
    {"loop_sustain", DiscreteValue::LOOP_SUSTAIN},
    {"no_loop", DiscreteValue::NO_LOOP},
    {"one_shot", DiscreteValue::ONE_SHOT},
    {"attack", DiscreteValue::ATTACK},
    {"release", DiscreteValue::RELEASE},
    {"first", DiscreteValue::RELEASE},
    {"legato", DiscreteValue::RELEASE},
    {"release_key", DiscreteValue::RELEASE}
};

DiscreteValue SamplerSchema::translated(const std::string& s) {
    auto it = discreteValues.find(s);
    if (it == discreteValues.end()) {
        printf("isn't discrete: %s\n", s.c_str());
        return DiscreteValue::NONE;
    }
    return it->second;
}

OpcodeType SamplerSchema::keyTextToType(const std::string& key, bool suppressErrorMessages) {
    Opcode opcode = SamplerSchema::translate(key, suppressErrorMessages);
    if (opcode == Opcode::NONE) {
        if (!suppressErrorMessages) {
            SQINFO("unknown opcode type %s", key.c_str());
        }
        return OpcodeType::Unknown;
    }
    auto typeIter = keyType.find(opcode);
    // OpcodeType type = keyType(opcode);
    if (typeIter == keyType.end()) {
        SQFATAL("unknown type for key %s", key.c_str());
        return OpcodeType::Unknown;
    }
    return typeIter->second;
}

// TODO: octaves
// TODO: IPN octaves
// TODO: upper case
std::pair<bool, int> SamplerSchema::convertToInt(SamplerErrorContext& err, const std::string& _s) {
    std::string s(_s);
    int noteName = -1;
    bool sharp = false;

    if (s.length() >= 2) {
        int firstChar = s[0];

        // They may not be legal? but we see pitches upper case sometimes
        if ((firstChar >= 'A' && firstChar <= 'G')) {
            firstChar -= ('A' - 'a');
        }
        if (firstChar >= 'a' && firstChar <= 'g') {
            switch (firstChar) {
                case 'a':
                    noteName = 9;
                    break;
                case 'b':
                    noteName = 11;
                    break;
                case 'c':
                    noteName = 0;
                    break;
                case 'd':
                    noteName = 2;
                    break;
                case 'e':
                    noteName = 4;
                    break;
                case 'f':
                    noteName = 5;
                    break;
                case 'g':
                    noteName = 7;
                    break;
            }
#if 0
            noteName = firstChar - 'a';
            noteName -= 2;  // c is zero
            if (noteName < 0) {
                noteName += 12;         // b3 + 1 = c4
            }
#endif
        }
        if (noteName >= 0) {
            const int secondChar = s[1];
            if (secondChar == '#') {
                sharp = true;
            }
        }
    }

    if (noteName >= 0 && sharp) {
        s = s.substr(2);
    } else if (noteName >= 0) {
        s = s.substr(1);
    }

    try {
        int x = std::stoi(s);
        if (noteName >= 0) {
            x *= 12;               // number part is octave in this form
            x += (12 + noteName);  // 12 is c0 in midi

            if (sharp) {
                x += 1;
            }
        }
        return std::make_pair(true, x);
    } catch (std::exception&) {
        SQWARN("could not convert %s to Int", s.c_str());
        err.sawMalformedInput = true;
        return std::make_pair(false, 0);
    }
}

void SamplerSchema::compile(SamplerErrorContext& err, SamplerSchema::KeysAndValuesPtr results, SKeyValuePairPtr input) {
    Opcode opcode = translate(input->key, false);
    if (opcode == Opcode::NONE) {
        //  std::string e = std::string("could not translate opcode ") + input->key.c_str();
        err.unrecognizedOpcodes.insert(input->key);
        //SQWARN("could not translate opcode %s", input->key.c_str());
        return;
    }
    auto typeIter = keyType.find(opcode);
    if (typeIter == keyType.end()) {
        SQFATAL("could not find type for %s", input->key.c_str());
        assert(false);
        return;
    }

    const OpcodeType type = typeIter->second;

    ValuePtr vp = std::make_shared<Value>();
    vp->type = type;
    bool isValid = true;
    switch (type) {
        case OpcodeType::Int: {
            auto foo = convertToInt(err, input->value);
            if (!foo.first) {
                return;
            }
            vp->numericInt = foo.second;
        } break;
        case OpcodeType::Float:
            try {
                float x = std::stof(input->value);
                vp->numericFloat = x;
            } catch (std::exception&) {
                isValid = false;
                SQWARN("could not convert %s to float. key=%s", input->value.c_str(), input->key.c_str());
                err.sawMalformedInput = true;
                return;
            }
            break;
        case OpcodeType::String:
            vp->string = input->value;
            break;
        case OpcodeType::Discrete: {
            const DiscreteValue dv = translated(input->value);
            if (dv ==  DiscreteValue::NONE) {
                SQINFO("malformed discrete kb = %s, %s", input->key.c_str(), input->value.c_str());
                err.sawMalformedInput = true;
                return;
            }
 
            vp->discrete = dv;
        } break;
        default:
            assert(false);
    }
    if (isValid) {
        results->add(opcode, vp);
    }
}

SamplerSchema::KeysAndValuesPtr SamplerSchema::compile(SamplerErrorContext& err, const SKeyValueList& inputs) {
    SamplerSchema::KeysAndValuesPtr results = std::make_shared<SamplerSchema::KeysAndValues>();
    for (auto input : inputs) {
        compile(err, results, input);
    }
    return results;
}

SamplerSchema::Opcode SamplerSchema::translate(const std::string& s, bool suppressErrors) {
    auto entry = opcodes.find(s);
    if (entry == opcodes.end()) {
        auto find2 = unrecognized.find(s);
        if (find2 == unrecognized.end()) {
            unrecognized.insert({s});
            if (!suppressErrors) {
                SQWARN("!! unrecognized opcode %s\n", s.c_str());
            }
        }

        return Opcode::NONE;
    } else {
        return entry->second;
    }
}

std::set<std::string> SamplerSchema::freeTextFields = {
    "sample",
    "label_cc*",
    "label_key*",
    "default_path",
    "sw_label",
    "delay_filter",
    "filter_type",
    "global_label",
    "group_label",
    "image",
    "vendor_specific",
    "static_filter",
    "region_label",
    "master_label",
    "md5"

};

bool SamplerSchema::isFreeTextType(const std::string& key) {
    // string input("aasdf43");
    std::string stringToMatch = key;

    // dollar sign goes in because we don't tranlate #define
    static std::string matches("0123456789$");
    auto offset = key.find_first_of(matches);
    if (offset != std::string::npos) {
        stringToMatch = key.substr(0, offset) + '*';
    }

    auto it = freeTextFields.find(stringToMatch);
    return it != freeTextFields.end();
}

std::vector<std::string> SamplerSchema::_getKnownTextOpcodes() {
    std::vector<std::string> ret;
    for (auto x : opcodes) {
        const Opcode oc = x.second;
        auto type = keyType[oc];
        if (type == OpcodeType::String) {
            ret.push_back(x.first);
        }
    }
    return ret;
}

std::vector<std::string> SamplerSchema::_getKnownNonTextOpcodes() {
    std::vector<std::string> ret;
    for (auto x : opcodes) {
        const Opcode oc = x.second;
        auto type = keyType[oc];
        if (type != OpcodeType::String) {
            ret.push_back(x.first);
        }
    }
    return ret;
}