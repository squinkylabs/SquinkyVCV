#pragma once

#include <memory>
class KeyMapping;
using KeyMappingPtr = std::shared_ptr<KeyMapping>;
class KbdManager
{
public:
   KbdManager();
private:
    static void init();
    static KeyMappingPtr defaultMappings;
    static KeyMappingPtr userMappings;
};

using KbdManagerPtr = std::shared_ptr<KbdManager>;
