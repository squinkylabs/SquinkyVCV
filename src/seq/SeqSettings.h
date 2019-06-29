#pragma once


#include "ISeqSettings.h"

#include "rack.hpp"

#include <stdio.h>

class SeqSettings : public ISeqSettings
{
public:
    friend class GridMenuItem;
    SeqSettings(rack::engine::Module*);
    void invokeUI(rack::widget::Widget* parent) override;
    float getQuarterNotesInGrid() override;
private:
    rack::engine::Module* const module;

    float quarterNotesInGrid = 1;

};