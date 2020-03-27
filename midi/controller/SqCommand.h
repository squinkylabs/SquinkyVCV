#pragma once

#include <memory>
#include <string>

class MidiSequencer;
class SequencerWidget;
class MidiSequencer4;
class Sequencer4Widget;

using MidiSequencerPtr = std::shared_ptr<MidiSequencer>;
using MidiSequencer4Ptr = std::shared_ptr<MidiSequencer4>;
class SqCommand
{
public:
    virtual ~SqCommand() {}
    virtual void execute(MidiSequencerPtr seq, SequencerWidget* widget) = 0;
    virtual void undo(MidiSequencerPtr seq, SequencerWidget*) = 0;
    std::string name = "Seq++";
};

class Sq4Command
{
public:
    virtual ~Sq4Command() {}
    virtual void execute(MidiSequencer4Ptr seq, Sequencer4Widget* widget) = 0;
    virtual void undo(MidiSequencer4Ptr seq, Sequencer4Widget*) = 0;
    std::string name = "4X4";
};

using CommandPtr = std::shared_ptr<SqCommand>;
using Command4Ptr = std::shared_ptr<Sq4Command>;