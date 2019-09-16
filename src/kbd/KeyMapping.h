#pragma once

#include "Actions.h"
#include "SqKey.h"

#include <map>
#include <string>

class Actions;

class KeyMapping
{
public:
    /** 
     * If constructor fails, will return no mappings
     */
    KeyMapping(const std::string& configPath);
    Actions::action get(const SqKey&);
private:
   
    using container = std::map<SqKey, Actions::action>;

    container theMap;


    Actions::action parseAction(Actions&, json_t* binding);

};