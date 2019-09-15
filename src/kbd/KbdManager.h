#pragma once

#include <memory>
class KeyMapping;
using KeyMappingPtr = std::shared_ptr<KeyMapping>;
class KbdManager
{
public:
    static void init();
private:
    static KeyMappingPtr defaultMappings;
    static KeyMappingPtr userMappings;
};
