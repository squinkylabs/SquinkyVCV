#pragma once

class MidiSequencer;

class MidiKeyboardHandler
{
public:
    static bool handle(MidiSequencer* sequencer, unsigned key, unsigned mods);
};