#pragma once

#include <string>

class KeyMapping
{
public:
    /** 
     * If constructor fails, will return no mappings
     */
    KeyMapping(const std::string& configPath);

};