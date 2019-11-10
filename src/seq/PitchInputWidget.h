#pragma once

class InputPopupMenuParamWidget;

#include "InputControls.h"
#include <vector>
#include "rack.hpp"

class PitchInputWidget : public ::rack::OpaqueWidget
{
public:
    /**
     * param label is the label that goes beside the pitch entry UI.
     * param inputControls is where we must push the two "controls" we create
     */ 
    PitchInputWidget(
        const ::rack::math::Vec& pos,
        const ::rack::math::Vec& siz,
        const std::string& label, 
        bool relativePitch,
        std::vector<InputControl*>& inputControls);
private:
    class InputControlFacade : public InputControl
    {
    public:
        float getValue() const override;
        void setValue(float) override;
        void enable(bool enabled) override;
        ~InputControlFacade() override;
    
        void setCallback(std::function<void(void)>) override;
    };


    // We have two pitch inputs, and switch them up dependingon "scale relative" setting
    InputPopupMenuParamWidget* chromaticPitchInput = nullptr;
    InputPopupMenuParamWidget* scaleDegreesInput = nullptr;
};