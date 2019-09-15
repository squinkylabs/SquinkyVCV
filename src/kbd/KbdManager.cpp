#include "KbdManager.h"
#include "KeyMapping.h"

#include "asset.hpp"

void KbdManager::init()
{
    printf("\n\n settings patch = %s\n", rack::asset::settingsPath.c_str());
    printf("asset::user = %s\n\n", rack::asset::user("foo.json").c_str());
    fflush(stdout);
    // these statics are shared by all instances
    if (!defaultMappings) {
        std::string keymapPath =  rack::asset::user("seq_default_keys.json");
        defaultMappings = std::make_shared<KeyMapping>(keymapPath);
    }
    if (!userMappings) {
        printf("not reading real files yet\n");
        //std::string keymapPath =  rack::asset::user("seq_user_keys.json");
        //userMappings = std::make_shared<KeyMapping>(keymapPath);
    }
}

KeyMappingPtr KbdManager::defaultMappings;
KeyMappingPtr KbdManager::userMappings;