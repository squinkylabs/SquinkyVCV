#pragma once


#include "ISeqSettings.h"

#include "Rack.hpp"

#include <stdio.h>

class SeqSettings : public ISeqSettings
{
public:
    SeqSettings(rack::engine::Module*);
    void invokeUI(rack::widget::Widget* parent) override;
    float getSecondsInGrid() override;
private:
    rack::engine::Module* const module;

};