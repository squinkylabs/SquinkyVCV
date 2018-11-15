#pragma once

template <int N>
class MultiLag
{
public:
    void setAttack(float)
    {

    }
    void setRelease(float)
    {

    }
    void step(const float * buffer)
    {

    }
    float get(int index) const
    {
        assert(index < N);
        return memory[index];
    }
private:
    float memory[N] = {0};


};
