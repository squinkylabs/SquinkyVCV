#pragma once

#include "SqKey.h"

#include <functional>
#include <map>
#include <string>

class KeyMapping
{
public:
    /** 
     * If constructor fails, will return no mappings
     */
    KeyMapping(const std::string& configPath);
private:
    using action = std::function<void(void)>;
    using container = std::map<SqKey, action>;

    container theMap;


    action parseAction(json_t* binding);

};