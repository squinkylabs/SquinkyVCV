#pragma once

#include "IMidiPlayerHost.h"

class MidiAudition : public IMidiPlayerAuditionHost
{
public:
    MidiAudition(IMidiPlayerHostPtr h) : playerHost(h)
    {

    }
    void auditionNote(float pitch) override
    {

    }
private:
    IMidiPlayerHostPtr playerHost;

};