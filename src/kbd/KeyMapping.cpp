
#include "KeyMapping.h"
#include "SqKey.h"

#include "jansson.h"
#include "logger.hpp"

#include <stdio.h>

KeyMapping::KeyMapping(const std::string& configPath)
{
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
#if 0
                {
                    SqKeyPtr key2 = SqKey::parse(value);
                    const bool b = key < key2;

                    SqKey k = *key;
                    SqKey k2 = *key2;
                    bool c = k < k2;
                }
                #endif

                action act = parseAction(value);
                if (!act || !key) {
                    return;
                }
                theMap[*key] = act;
                fprintf(stderr, "ust added to map, size = %d\n", (int) theMap.size());
                fprintf(stderr, "finish me\n");
            }
        } else {
            fprintf(stderr, "bindings is not an array\n");
        }
    } else {
        fprintf(stderr, "bindings not found at root\n");
    }

    fprintf(stderr, "must parse json\n");

    json_decref(mappingJson);
    fclose(file);
};


KeyMapping::action KeyMapping::parseAction(json_t* binding)
{
    fprintf(stderr, "parse action is fake\n");
    return []() {};
}