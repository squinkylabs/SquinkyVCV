
#include "KeyMapping.h"
#include "SqKey.h"

#include "jansson.h"
#include "logger.hpp"

#include <stdio.h>

KeyMappingPtr KeyMapping::make(const std::string& configPath)
{
    KeyMappingPtr ret( new KeyMapping(configPath));
    if (!ret->valid()) {
        ret.reset();
    }
    return ret;

}

bool KeyMapping::valid() const
{
    return !theMap.empty();
}

KeyMapping::KeyMapping(const std::string& configPath)
{
    Actions actions;
    fprintf(stderr, "key mapping::key %s\n", configPath.c_str());
    FILE *file = fopen(configPath.c_str(), "r");
    if (!file) {
        fprintf(stderr, "could not open file: %s\n", configPath.c_str());
        return;
    }

    json_error_t error;
	json_t *mappingJson = json_loadf(file, 0, &error);
    if (!mappingJson) {
        fprintf(stderr, "could not parse json\n"); fflush(stdout);
    }

    json_t* bindings = json_object_get(mappingJson, "bindings");
    if (bindings) {
        if (json_is_array(bindings)) {
            size_t index;
            json_t* value;
            json_array_foreach(bindings, index, value) {
                SqKeyPtr key = SqKey::parse(value);
                Actions::action act = parseAction(actions, value);
                if (!act || !key) {
                    fprintf(stderr, "skipping bad entry: %s\n", json_dumps(value, 0));
                } else {
                    //fprintf(stderr, "adding entry to map code = %d\n", key->key);

                    if (theMap.find(*key) != theMap.end()) {
                        fprintf(stderr, "duplicate key mapping %s\n", json_dumps(value, 0));
                    }
                    theMap[*key] = act;
                }
            }
        } else {
            fprintf(stderr, "bindings is not an array\n");
        }
    } else {
        fprintf(stderr, "bindings not found at root\n");
    }

    std::set<int> ignoreCodes;
    json_t* ignoreCase = json_object_get(mappingJson, "ignore_case");
    if (ignoreCase) {
        if (json_is_array(ignoreCase)) {
            size_t index;
            json_t* value;
            json_array_foreach(ignoreCase, index, value) {
                if (json_is_string(value)) {
                    std::string key = json_string_value(value);
                    int code = SqKey::parseKey(key);
                    ignoreCodes.insert(code);
                } else {
                    fprintf(stderr, "bad key in ignore_case: %s\n", json_dumps(value, 0));
                }
            }
        } else {
            fprintf(stderr, "ignoreCase is not an array\n");
        }
    }

    processIgnoreCase(ignoreCodes);
    json_decref(mappingJson);
    fclose(file);
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
            SqKey newKey(key.key, key.ctrl, true);
#ifdef _DEBUG
            const size_t first = theMap.size();
#endif
            theMap[newKey] = it.second;
            assert(theMap.size() == (first + 1));
        }
    }
}

Actions::action KeyMapping::parseAction(Actions& actions, json_t* binding)
{
    json_t* keyJ = json_object_get(binding, "action");
    if (!keyJ) {
        fprintf(stderr, "binding does not have action field: %s\n",
            json_dumps(keyJ, 0));
        return nullptr;
    }
    if (!json_is_string(keyJ)) {
        fprintf(stderr, "binding action is not a string");
        return nullptr;
    }

    std::string actionString = json_string_value(keyJ);
    auto act = actions.getAction(actionString);
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