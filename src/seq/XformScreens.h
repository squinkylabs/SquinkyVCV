#pragma once

#include "InputScreen.h"
class XformInvert : public InputScreen
{
public:
    XformInvert(const ::rack::math::Vec& pos,
        const ::rack::math::Vec& size,
        MidiSequencerPtr seq,
        std::function<void(bool)> _dismisser);
    void execute() override;
};