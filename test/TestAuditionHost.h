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

    void reset()
    {
        notes.clear();
    }

    int count() const
    {
        return int(notes.size());
    }

    std::vector<float> notes;
};