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
       // const int numInputs = (int) dut.inputs.size();
        const int numInputs = dut.NUM_INPUTS;
        assert(numInputs < 20);
       

        for (inputState.reset(numInputs); !inputState.isDone(); inputState.next()) {
            inputState.dump();
            inputState.setState(dut.inputs);
        }
      

    }
private:
    T & dut;
    BitCounter inputState;

};