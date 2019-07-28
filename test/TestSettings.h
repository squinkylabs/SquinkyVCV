#pragma once


#include <assert.h>
#include "ISeqSettings.h"

class TestSettings : public ISeqSettings
{
public:
    virtual void invokeUI(rack::widget::Widget* parent)
    {
    }
    virtual float getQuarterNotesInGrid()
    {
        return 0;
    }
    virtual bool snapToGrid()
    {
        return false;
    }

    /**
     * if snap to grid,will quantize the passed value to the current grid.
     * otherwise does nothing.
     * will not quantize smaller than a grid.
     */
    virtual float quantize(float x, bool allowZero)
    {
       // assert(false);
        //return 0;
        return x;
    }
};