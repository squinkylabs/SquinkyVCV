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
    bool snapToGrid() override;
private:
    rack::engine::Module* const module;

    enum class Grids
    {
        quarter,
        eighth,
        sixteenth
    };

    Grids curGrid = Grids::quarter;
    bool snapEnabled = true;

    static float grid2Time(Grids);
    rack::ui::MenuItem* makeSnapItem();
};