#pragma once
#include "LadderFilter.h"

template <typename T>
class LadderFilterBank
{
public:
    void stepn();
    void step();
private:
    LadderFilter<T> filters[16];
};

template <typename T>
void LadderFilterBank<T>::stepn()
{

}

template <typename T>
void LadderFilterBank<T>::step()
{
    
}