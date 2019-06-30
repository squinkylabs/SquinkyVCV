#pragma once


#include "ISeqSettings.h"

#include "rack.hpp"

#include <stdio.h>

class SeqSettings : public ISeqSettings
{
public:
    friend class GridMenuItem;
    friend class GridItem;
    SeqSettings(rack::engine::Module*);
    void invokeUI(rack::widget::Widget* parent) override;
    float getQuarterNotesInGrid() override;
private:
    rack::engine::Module* const module;

    enum class Grids {
        quarter,
        eighth,
        sixteenth
    };

    Grids curGrid = Grids::quarter;

    static float grid2Time(Grids);
};