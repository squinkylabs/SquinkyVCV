#pragma once


#include <assert.h>
#include "ISeqSettings.h"

class TestSettings : public ISeqSettings
{
public:
    void invokeUI(rack::widget::Widget* parent) override
    {
    }
    float getQuarterNotesInGrid() override
    {
        return 0;
    }
    bool snapToGrid() override
    {
        return false;
    }
    bool snapDurationToGrid() override
    {
        return false;
    }

    /**
     * if snap to grid,will quantize the passed value to the current grid.
     * otherwise does nothing.
     * will not quantize smaller than a grid.
     */
    float quantize(float x, bool allowZero) override
    {
       // assert(false);
        //return 0;
        return x;
    }

    float articulation() override
    {
        return false;
    }
};