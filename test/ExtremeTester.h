#pragma once

#include <vector>

class BitCounter
{
public:
    void reset(int size)
    {
        done = false;
        state.resize(size);
        for (int i = 0; i < size; ++i) {
            state[i] = 0;
        }
    }
    bool atMax() const
    {
        for (int i = 0; i < state.size(); ++i) {
            if (state[i] == 0) {
                return false;
            }
        }
        return true;
    }
    bool isDone() const
    {
        return done;
    }
    void next()
    {
        if (atMax()) {
            done = true;
            return;
        }
        state[0]++;
        for (int i = 0; i < state.size(); ++i) {
            if (state[i] > 1) {
                state[i] = 0;
                ++state[i + 1];
            }
        }
       
    }
    void dump()
    {
        printf("State: ");
        for (int i = (int)state.size() - 1; i >= 0; --i) {
            printf("%d ", state[i]);
        }
        printf("\n");
    }

    template <typename Q>
    void setState(std::vector<Q> testSignal)
    {
        for (int i = (int) state.size() - 1; i >= 0; --i) {
            testSignal[i].value = state[i] > 0 ? 10.f : -10.f;
           
        }
    }
private:
    std::vector<int> state;
    bool done;

};

template <typename T>
class ExtremeTester
{
public:
    ExtremeTester(T& t) : dut(t)
    {

    }
    void test()
    {
      
        const int numInputs = dut.NUM_INPUTS;
        const int numParams = dut.NUM_PARAMS;
        const int numOutputs = dut.NUM_OUTPUTS;
        assert(numInputs < 20);
       
        BitCounter inputState;
        BitCounter paramsState;
        for (inputState.reset(numInputs); !inputState.isDone(); inputState.next()) {
            inputState.setState(dut.inputs);
            inputState.dump();
            for (paramsState.reset(numInputs); !paramsState.isDone(); paramsState.next()) {
                paramsState.setState(dut.params);
                for (int i=0; i < 100; ++i) {
                    dut.step();
                    for (int j = 0; j < numOutputs; ++j) {
                        const float out = dut.outputs[j].value;
                        assert(out > -10);
                        assert(out < 10);
                    }
                }
            }

        }
      

    }
private:
    T & dut;
   

};