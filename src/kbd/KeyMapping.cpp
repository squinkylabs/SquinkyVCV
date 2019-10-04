
#include "KeyMapping.h"
#include "SqKey.h"

#include "jansson.h"
#include "logger.hpp"

#include <stdio.h>
#include <sstream>

KeyMappingPtr KeyMapping::make(const std::string& configPath)
{
    KeyMappingPtr ret;
    try {
        ret.reset( new KeyMapping(configPath));
    } catch( std::exception& ex) {
        std::string errorStr = std::string("Error detected parsing key mapping: ") + ex.what();
        WARN(errorStr.c_str());
        assert(!ret);
    }
    return ret;
}

bool KeyMapping::useDefaults() const
{
    return _useDefaults;
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
    INFO("parsing key mapping: %s\n", configPath.c_str());
    FILE *file = fopen(configPath.c_str(), "r");
    if (!file) {
        std::string errorStr("could not open file mapping: ");
        errorStr += configPath;
        throw (std::runtime_error(errorStr));
    }

    json_error_t error;
	json_t *mappingJson = json_loadf(file, 0, &error);
    if (!mappingJson) {
        std::stringstream s;
        s << "JSON parsing error at ";
        s <<  error.line << ":" << error.column;
        s << " " << error.text;
        fclose(file);
        throw (std::runtime_error(s.str()));
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
                Actions::action act = parseAction(actions, value);
                if (!act || !key) {
                     std::stringstream s;
                     s << "Bad binding entry (" << json_dumps(value, 0) << ")";
                     throw (std::runtime_error(s.str()));
                }
                if (theMap.find(*key) != theMap.end()) {
                    std::stringstream s;
                    s << "duplicate key mapping: " << json_dumps(value, 0);
                    throw (std::runtime_error(s.str()));
                }
                theMap[*key] = act;
            }
        } else {
            throw (std::runtime_error("bindings is not an array"));
        }
    } else {
        throw (std::runtime_error("bindings not found at root"));
    }

    //*********** ignore case ************
    std::set<int> ignoreCodes;
    json_t* ignoreCase = json_object_get(mappingJson, "ignore_case");
    if (ignoreCase) {
        if (!json_is_array(ignoreCase)) {
             throw (std::runtime_error("ignoreCase is not an array"));
        }
        size_t index;
        json_t* value;
        json_array_foreach(ignoreCase, index, value) {
            if (!json_is_string(value)) {
                std::stringstream s;
                s << "bad key in ignore_case: " <<  json_dumps(value, 0);
                throw (std::runtime_error(s.str()));
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
            throw (std::runtime_error("use_defaults is not an array"));
        }
        _useDefaults = json_is_true(useDefaultsJ);
    }
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
        WARN("binding does not have action field: %s\n", json_dumps(keyJ, 0));
        return nullptr;
    }
    if (!json_is_string(keyJ)) {
        WARN("binding action is not a string: %s\n", json_dumps(keyJ, 0));
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