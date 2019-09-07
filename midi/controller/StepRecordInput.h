#pragma once

#include "RingBuffer.h"
#include <assert.h>

class RecordInputData
{
public:
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

    static const int maxNotes = 16;
    SqRingBuffer<RecordInputData, maxNotes> buffer;
    bool gateWasHigh[maxNotes] = {0};
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
    assert(p);
    bool ret = false;
    if (!buffer.empty()) {
        *p = buffer.pop();
        ret = true;
    }
    return ret;
}

template <typename TPort>
inline void StepRecordInput<TPort>::step()
{
    bool highBefore = false;
    bool highAfter = false;
    for (int i = 0; i < maxNotes; ++i) {
        highBefore |= gateWasHigh[i];
        if (i < gate.channels) {
            bool b = gate.voltages[i] > 5;      // TODO: schmidt
            highAfter |= b;
            if (b != gateWasHigh[i]) {
                if (b) {
                    // gate just went high
                    RecordInputData data;
                    data.type = RecordInputData::Type::noteOn;
                    data.pitch = cv.voltages[i];
                    buffer.push(data);
                } else {
                    // gate just went low
                   // assert(false);
                }
                gateWasHigh[i] = b;
            }
        }
    }
    if (highBefore && !highAfter) {
        RecordInputData data;
        data.type = RecordInputData::Type::allNotesOff;
        buffer.push(data);
    }
}