#include "ActionContext.h"
#include "KbdManager.h"
#include "KeyMapping.h"
#include "StepRecorder.h"

#include "asset.hpp"
#include "event.hpp"

#include <GLFW/glfw3.h>

#include <unistd.h>

extern ::rack::plugin::Plugin *pluginInstance;

KeyMappingPtr KbdManager::defaultMappings;
KeyMappingPtr KbdManager::userMappings;

KbdManager::KbdManager()
{
    init();
    stepRecorder = std::make_shared<StepRecorder>();
}

void KbdManager::init()
{
    // these statics are shared by all instances
    if (!defaultMappings) {
        std::string keymapPath = ::rack::asset::plugin(pluginInstance, "res/seq_default_keys.json");
        defaultMappings = KeyMapping::make(keymapPath);
    }
    if (!userMappings) {
        std::string keymapPath =  ::rack::asset::user("seq_user_keys.json"); 
        userMappings = KeyMapping::make(keymapPath);
    }
}

bool KbdManager::shouldGrabKeys() const
{
    bool ret = true;
    if (userMappings) {
        ret = userMappings->grabKeys();
    }
    return ret;
}

bool KbdManager::handle(MidiSequencerPtr sequencer, unsigned keyCode, unsigned mods)
{

    bool handled = false;
    const bool shift = (mods & GLFW_MOD_SHIFT);
    const bool ctrl = (mods & RACK_MOD_CTRL);        // this is command on mac
    const bool alt = (mods && GLFW_MOD_ALT);
    SqKey key(keyCode, ctrl, shift, alt);

   // fprintf(stderr, "\n** KbdManager::handle code=%d mods=%d\n", keyCode, mods); 
   // fprintf(stderr, " shift=%d, ctrl=%d\n", shift, ctrl); fflush(stderr);

    assert(defaultMappings);

    bool suppressDefaults = false;
    ActionContext ctx(sequencer, stepRecorder);
    if (userMappings) {
        Actions::action act = userMappings->get(key);
        if (act) {
            act(ctx);
            handled = true;
        }
        suppressDefaults = !userMappings->useDefaults();

    }

    if (!handled && !suppressDefaults) {
        Actions::action act = defaultMappings->get(key);
        if (act) {
            act(ctx);
            handled = true;
        }
    }
    // fprintf(stderr, "KbdManager::handle ret %d\n", handled);

    return handled;
}

void KbdManager::onUIThread(std::shared_ptr<Seq<WidgetComposite>> seqComp, MidiSequencerPtr sequencer)
{
    stepRecorder->onUIThread(seqComp, sequencer);
}