
#include "KeyMapping.h"

#include "jansson.h"
#include "logger.hpp"

#include <stdio.h>

KeyMapping::KeyMapping(const std::string& configPath)
{
    printf("key mapping::key %s\n", configPath.c_str()); fflush(stdout);
    FILE *file = fopen(configPath.c_str(), "r");
    if (!file) {
        printf("didn't find %s\n", configPath.c_str());
        fflush(stdout);
        return;
    }

    json_error_t error;
	json_t *mappingJson = json_loadf(file, 0, &error);
    if (!mappingJson) {
        printf("could not open json\n"); fflush(stdout);
        return;
    }

    json_t* bindings = json_object_get(mappingJson, "bindings");
    if (bindings) {
        if (json_is_array(bindings)) {
            size_t index;
            json_t* value;
            json_array_foreach(bindings, index, value) {
                printf("finish me\n"); fflush(stdout);
            }

        } else {
            printf("bindings is not an array\n"); fflush(stdout);
        }
    } else {
        printf("bindings not found at root\n"); fflush(stdout);
    }

    printf("mast parse json\n"); fflush(stdout);
    fflush(stdout);

    json_decref(mappingJson);
    fclose(file);
};