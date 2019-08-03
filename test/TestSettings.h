#pragma once


#include <assert.h>
#include "ISeqSettings.h"
#include "TimeUtils.h"

class TestSettings : public ISeqSettings
{
public:
    void invokeUI(rack::widget::Widget* parent) override
    {
    }
    float getQuarterNotesInGrid() override
    {
        return _quartersInGrid;
    }
    bool snapToGrid() override
    {
        return _snapToGrid;
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
    float quantize(float time, bool allowZero) override
    {
        auto quantized = time;
        if (snapToGrid()) {
            quantized = (float) TimeUtils::quantize(time, getQuarterNotesInGrid(), allowZero);
        }
        return quantized;
    }

    float articulation() override
    {
        return _articulation;
    }

    float _articulation = 1;
    float _quartersInGrid = .25;
    bool _snapToGrid = true;
};