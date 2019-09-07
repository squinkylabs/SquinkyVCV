#pragma once

#include "RingBuffer.h"

class RecordInputData
{
    enum class Type {noteOn, allNotesOff };

    float pitch=0;
    Type type = Type::allNotesOff;
};

template <typename TPort>
class StepRecordInput
{
public:
    StepRecordInput(TPort& cv, TPort& gate);

    /**
     * returns true if data retrieved. Data goes to *p.
     * OK to call from any thread, although typically UI thread.
     */
    bool poll(RecordInputData* p);

    /**
     * Will be called from the audio thread. 
     */
    void step();
private:
    TPort& cv;
    TPort& gate;

    SqRingBuffer<RecordInputData, 16> buffer;
};

template <typename TPort>
inline StepRecordInput<TPort>::StepRecordInput(TPort& cv, TPort& gate) :
    cv(cv),
    gate(gate)
{
}

template <typename TPort>
inline bool StepRecordInput<TPort>::poll(RecordInputData* p)
{
    return false;
}

template <typename TPort>
inline void StepRecordInput<TPort>::step()
{

}