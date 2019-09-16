#include "KbdManager.h"
#include "KeyMapping.h"

#include "asset.hpp"
#include "event.hpp"

#include <GLFW/glfw3.h>

extern rack::plugin::Plugin *pluginInstance;

KeyMappingPtr KbdManager::defaultMappings;
KeyMappingPtr KbdManager::userMappings;

KbdManager::KbdManager()
{
    init();
}

void KbdManager::init()
{
    printf("\n\n settings patch = %s\n", rack::asset::settingsPath.c_str());
    printf("asset::user = %s\n\n", rack::asset::user("foo.json").c_str());
    fflush(stdout);
    // these statics are shared by all instances
    if (!defaultMappings) {
        std::string keymapPath = rack::asset::plugin(pluginInstance, "res/seq_default_keys.json");
        //std::string keymapPath =  rack::asset::user("seq_default_keys.json");
        defaultMappings = std::make_shared<KeyMapping>(keymapPath);
    }
    if (!userMappings) {
        printf("not reading real files yet\n");
        //std::string keymapPath =  rack::asset::user("seq_user_keys.json");
        //userMappings = std::make_shared<KeyMapping>(keymapPath);
    }
}


 bool KbdManager::handle(MidiSequencerPtr sequencer, unsigned keyCode, unsigned mods)
 {
    fprintf(stderr, "KbdManager::handle code=%d mods=%d\n", keyCode, mods); fflush(stderr);
    bool handled = false;
    const bool shift = (mods & GLFW_MOD_SHIFT);
    const bool ctrl = (mods & RACK_MOD_CTRL);        // this is command on mac
    //const bool alt = (mods && GLFW_MOD_ALT);
    SqKey key(keyCode, ctrl, shift);
   //  fprintf(stderr, "b KbdManager::handle\n"); fflush(stderr);
    
    assert(defaultMappings);
    Actions::action act = defaultMappings->get(key);
    fprintf(stderr, "v KbdManager::handle act = %d\n", bool(act)); fflush(stderr);
    if (act) {
        fprintf(stderr, "calling act\n");
       act(sequencer);
       handled = true;
    }

    return handled;
 }