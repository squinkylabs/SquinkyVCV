
#include "KeyMapping.h"
#include "SqKey.h"
#include "rack.hpp"


//#include <system_error>
#include <stdio.h>
#include "SqStream.h"

KeyMappingPtr KeyMapping::make(const std::string& configPath)
{
    KeyMappingPtr ret;
    ret.reset( new KeyMapping(configPath));
    if (!ret->isValid()) {
        ret.reset();
    }
    return ret;
}

bool KeyMapping::useDefaults() const
{
    return _useDefaults;
}


bool KeyMapping::grabKeys() const
{
    return _grabKeys;
}

class JSONcloser
{
public:
    JSONcloser(json_t* j, FILE* f) :
        mappingJson(j),
        fp(f)
    {

    }
    ~JSONcloser()
    {
        json_decref(mappingJson);
        fclose(fp);
    }
private:
    json_t * const mappingJson;
    FILE* const fp;
};

KeyMapping::KeyMapping(const std::string& configPath)
{
    Actions actions;
    // INFO("parsing key mapping: %s\n", configPath.c_str());
    FILE *file = fopen(configPath.c_str(), "r");
    if (!file) {
        return;
    }

    json_error_t error;
	json_t *mappingJson = json_loadf(file, 0, &error);
    if (!mappingJson) {
        SqStream s;
        s.add("JSON parsing error at ");
        s.add(error.line);
        s.add(":");
        s.add(error.column);
        s.add(" ");
        s.add(error.text);
        fclose(file);
        INFO(s.str().c_str());
        return;
    }
    JSONcloser cl(mappingJson, file);

    //*********** bindings **************

    json_t* bindings = json_object_get(mappingJson, "bindings");
    if (bindings) {
        if (json_is_array(bindings)) {
            size_t index;
            json_t* value;
            json_array_foreach(bindings, index, value) {
                SqKeyPtr key = SqKey::parse(value);
                // DEBUG("in init key=%d ctrl=%d shift=%d alt=%d", key->key, key->ctrl, key->shift, key->alt);
                Actions::action act = parseAction(actions, value);
                if (!act || !key) {
                    SqStream s;
                    s.add("Bad binding entry (");
                    s.add(json_dumps(value, 0));
                    s.add(")");
                    INFO(s.str().c_str());
                    return;
                }
                if (theMap.find(*key) != theMap.end()) {
                    SqStream s;
                    s.add("duplicate key mapping: ");
                    s.add(json_dumps(value, 0));
                    INFO(s.str().c_str());
                    return;
                }
                theMap[*key] = act;
            }
        } else {
            //throw (std::runtime_error("bindings is not an array"));
            INFO("bindings is not an array");
            return;
        }
    } else {
        INFO("bindings not found at root");
        return;
    }

    // DEBUG("Keyboard map has %d entries from JSON", theMap.size());

    //*********** ignore case ************
    std::set<int> ignoreCodes;
    json_t* ignoreCase = json_object_get(mappingJson, "ignore_case");
    if (ignoreCase) {
        if (!json_is_array(ignoreCase)) {
            INFO("ignoreCase is not an array");
            return;
        }
        size_t index;
        json_t* value;
        json_array_foreach(ignoreCase, index, value) {
            if (!json_is_string(value)) {
                SqStream s;
                s.add("bad key in ignore_case: ");
                s.add(json_dumps(value, 0));
                INFO(s.str().c_str());
                return;
            }
            std::string key = json_string_value(value);
            int code = SqKey::parseKey(key);
            ignoreCodes.insert(code);
        }
    }

    processIgnoreCase(ignoreCodes);

    //************ other top level props
    _useDefaults = true;
    json_t* useDefaultsJ = json_object_get(mappingJson, "use_defaults");
    if (useDefaultsJ) {
        if (!json_is_boolean(useDefaultsJ)) {
            SqStream s;
            s.add("use_defaults is not true or false, is");
            s.add(json_dumps(useDefaultsJ, 0));
            INFO(s.str().c_str());
            return;
        }
        _useDefaults = json_is_true(useDefaultsJ);
    }

    _grabKeys = true;
    json_t* grabKeysJ = json_object_get(mappingJson, "grab_keys");
    if (grabKeysJ) {
        if (!json_is_boolean(grabKeysJ)) {
            SqStream s;
            s.add("grab_keys is not true or false, is");
            s.add(json_dumps(grabKeysJ, 0));
            INFO(s.str().c_str());
            return;
        }
        _grabKeys = json_is_true(grabKeysJ);
    }
    _isValid = true;
};

void KeyMapping::processIgnoreCase(const std::set<int>& codes)
{
    // look through the mapping for non-shifted key that matches code.
    // If found, add the shifted version.
    for (auto it : theMap) {
        //SqKey& key = it->first;
        auto k = it.first;
        SqKey& key = k;
    
        if (!key.shift && (codes.find(key.key) != codes.end())) {
            SqKey newKey(key.key, key.ctrl, true, key.alt);
            theMap[newKey] = it.second;
        }
    }
}

Actions::action KeyMapping::parseAction(Actions& actions, json_t* binding)
{
    json_t* keyJ = json_object_get(binding, "action");
    if (!keyJ) {
        WARN("binding does not have action field: %s\n", json_dumps(keyJ, 0));
        return nullptr;
    }
    if (!json_is_string(keyJ)) {
        WARN("binding action is not a string: %s\n", json_dumps(keyJ, 0));
        return nullptr;
    }

    std::string actionString = json_string_value(keyJ);
    auto act = actions.getAction(actionString);
    // DEBUG("parse action %s returned %p\n", actionString.c_str(), act);
    return act;
}

Actions::action KeyMapping::get(const SqKey& key)
{
    auto it = theMap.find(key);
    if (it == theMap.end()) {
        return nullptr;
    } else {
        return it->second;
    }
}