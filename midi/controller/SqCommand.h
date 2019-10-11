#pragma once

#include <memory>
#include <string>

class MidiSequencer;
class SequencerWidget;
using MidiSequencerPtr = std::shared_ptr<MidiSequencer>;
class SqCommand
{
public:
    virtual ~SqCommand() {}
    virtual void execute(MidiSequencerPtr seq, SequencerWidget* widget) = 0;
    virtual void undo(MidiSequencerPtr seq, SequencerWidget*) = 0;
    std::string name = "Seq++";
};

using CommandPtr = std::shared_ptr<SqCommand>;