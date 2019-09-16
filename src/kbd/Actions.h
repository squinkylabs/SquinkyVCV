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

    /**
     * all of the actions
     */
    static void insertDefault(MidiSequencerPtr);
    static void insertWhole(MidiSequencerPtr);
    static void insertHalf(MidiSequencerPtr);
    static void insertQuarter(MidiSequencerPtr);
    static void insertEighth(MidiSequencerPtr);
    static void insertSixteenth(MidiSequencerPtr);
    static void insertWholeAdvance(MidiSequencerPtr);
    static void insertHalfAdvance(MidiSequencerPtr);
    static void insertQuarterAdvance(MidiSequencerPtr);
    static void insertEighthAdvance(MidiSequencerPtr);
    static void insertSixteenthAdvance(MidiSequencerPtr);
    static void moveLeftNormal(MidiSequencerPtr);
    static void moveUpNormal(MidiSequencerPtr);
    static void moveDownNormal(MidiSequencerPtr);
    static void moveRightNormal(MidiSequencerPtr);
};