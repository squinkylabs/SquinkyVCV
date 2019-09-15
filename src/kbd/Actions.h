#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>

class MidiSequencer;
using MidiSequencerPtr = std::shared_ptr<MidiSequencer>;

class Actions
{
public:
    Actions();
    using action = std::function<void(MidiSequencerPtr)>;
    action getAction(const std::string& name);
private:

    std::map< std::string, action> _map;

    static bool insertDefault(MidiSequencerPtr);
    
};