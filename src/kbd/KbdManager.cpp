#include "rack.hpp"
#include "ActionContext.h"
#include "KbdManager.h"
#include "KeyMapping.h"
#include "StepRecorder.h"


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
    // DEBUG("KbdManager::handle code %d (q = %d) mods = %x", keyCode, GLFW_KEY_Q, mods);
    bool handled = false;
    const bool shift = (mods & GLFW_MOD_SHIFT);
    const bool ctrl = (mods & RACK_MOD_CTRL);        // this is command on mac
    const bool alt = (mods & GLFW_MOD_ALT);
    SqKey key(keyCode, ctrl, shift, alt);
    // DEBUG("mods parsed to ctrl:%d shift:%d alt:%d", ctrl, shift, alt);

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
    // DEBUG("in mgs, handled = %d", handled);
    return handled;
}

void KbdManager::onUIThread(std::shared_ptr<Seq<WidgetComposite>> seqComp, MidiSequencerPtr sequencer)
{
    stepRecorder->onUIThread(seqComp, sequencer);
}