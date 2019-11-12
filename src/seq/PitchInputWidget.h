#pragma once

class InputPopupMenuParamWidget;

#include "InputControls.h"
#include <vector>
#include "rack.hpp"

class PitchInputWidget : public InputControl, public ::rack::OpaqueWidget
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
        bool relativePitch);

    /** In chromatic mode only transposeSemis() may be called.
     * In scale relative mode, only transposeDegrees may be called.
     */
    bool isChromaticMode() const;
    int transposeDegrees() const;
    int transposeSemis() const;
    int transposeOctaves() const;

    /**
     * Implement enough of InputControls
     * so we can pretend to be one.
     */
    float getValue() const override;
    void setValue(float) override;
    void enable(bool enabled) override;
    void setCallback(std::function<void(void)>) override;
private:
/*
    class InputControlFacade : public InputControl
    {
    public:
        float getValue() const override;
        void setValue(float) override;
        void enable(bool enabled) override;
        ~InputControlFacade() override;
    
        void setCallback(std::function<void(void)>) override;
    };
*/

    InputPopupMenuParamWidget* octaveInput = nullptr;
    // We have two pitch inputs, and switch them up depending on "scale relative" setting
    InputPopupMenuParamWidget* chromaticPitchInput = nullptr;
    InputPopupMenuParamWidget* scaleDegreesInput = nullptr;
    CheckBox* keepInScale = nullptr;
    bool chromatic = true;

    std::function<void(void)> chromaticCb = nullptr;

    // ********************  constructor helpers ********************
    void addMainLabel(const std::string& labelText, const ::rack::math::Vec& pos);
    void addOctaveControl(const ::rack::math::Vec& pos);
    void addChromaticSemisControl(const ::rack::math::Vec& pos);
    void addScaleDegreesControl(const ::rack::math::Vec& pos);
    void addScaleRelativeControl(const ::rack::math::Vec& pos);

    ::rack::ui::Label* addLabel(const ::rack::math::Vec& v, const char* str, const NVGcolor& color =  UIPrefs::XFORM_TEXT_COLOR);

    void setChromatic(bool mode);
       // add chromatic semi

    // add scale degrees

    // add checkbox
};