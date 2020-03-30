#pragma once
#include "LadderFilter.h"

template <typename T>
class LadderFilterBank
{
public:
private:
    LadderFilter<T> filters[16];
};