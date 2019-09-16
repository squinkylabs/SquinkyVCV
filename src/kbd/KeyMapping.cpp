
#include "KeyMapping.h"
#include "SqKey.h"

#include "jansson.h"
#include "logger.hpp"

#include <stdio.h>

KeyMapping::KeyMapping(const std::string& configPath)
{
    Actions actions;
    fprintf(stderr, "key mapping::key %s\n", configPath.c_str());
    FILE *file = fopen(configPath.c_str(), "r");
    if (!file) {
        fprintf(stderr, "didn't find %s\n", configPath.c_str());
        return;
    }

    json_error_t error;
	json_t *mappingJson = json_loadf(file, 0, &error);
    if (!mappingJson) {
        fprintf(stderr, "could not open json\n"); fflush(stdout);
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
                    theMap[*key] = act;
                }
            }
        } else {
            fprintf(stderr, "bindings is not an array\n");
        }
    } else {
        fprintf(stderr, "bindings not found at root\n");
    }

    json_decref(mappingJson);
    fclose(file);
};

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
    fprintf(stderr, "looking for key code %d map size = %d\n", key.key, int(theMap.size()));
    auto it = theMap.find(key);
    if (it == theMap.end()) {
         fprintf(stderr, "didn't find\n");
        return nullptr;
    } else {
         fprintf(stderr, "found\n");
        return it->second;
    }
}