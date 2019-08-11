#pragma once

#include "IMidiPlayerHost.h"

#include <vector>

class TestAuditionHost : public IMidiPlayerAuditionHost
{
public:
    void auditionNote(float p) override
    {
        notes.push_back(p);
    }

    std::vector<float> notes;
};