#pragma once

#include "Actions.h"
#include "SqKey.h"

#include <map>
#include <set>
#include <string>

class Actions;
class KeyMapping;

using KeyMappingPtr = std::shared_ptr<KeyMapping>;

class KeyMapping
{
public:
    /** 
     * If constructor fails, will return no mappings.
     * will print diagnostic errors
     */
    Actions::action get(const SqKey&);

    static KeyMappingPtr make(const std::string& configPath);
    bool useDefaults() const;
    bool grabKeys() const;

    bool isValid() const {
        return _isValid;
    }

private:
    /**
     * returns with !isValid if parsing errors
     */
    KeyMapping(const std::string& configPath);
    using container = std::map<SqKey, Actions::action>;
    container theMap;
    Actions::action parseAction(Actions&, json_t* binding);
    void processIgnoreCase(const std::set<int>& ignoreCodes);

    bool _useDefaults = true;
    bool _grabKeys = true;
    bool _isValid = false;
};