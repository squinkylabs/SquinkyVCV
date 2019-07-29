#pragma once


#include "ISeqSettings.h"

#include "rack.hpp"

#include <stdio.h>

class SeqSettings : public ISeqSettings
{
public:
    friend class GridMenuItem;
    friend class GridItem;
    friend class SequencerSerializer;

    SeqSettings(rack::engine::Module*);
    void invokeUI(rack::widget::Widget* parent) override;

    /**
     * Grid related settings
     */
    float getQuarterNotesInGrid() override;
    bool snapToGrid() override;
    bool snapDurationToGrid() override;
    float quantize(float, bool allowZero) override;

    float articulation() override;
private:
    rack::engine::Module* const module;

    enum class Grids
    {
        quarter,
        eighth,
        sixteenth
    };

    std::string getGridString() const;
    static std::vector<std::string> getGridLabels();
    static Grids gridFromString(const std::string& s);

    Grids curGrid = Grids::quarter;
    bool snapEnabled = true;
    bool snapDurationEnabled = false;

    float articulationValue = .85;

    static float grid2Time(Grids);
    rack::ui::MenuItem* makeSnapItem();
    rack::ui::MenuItem* makeSnapDurationItem();
};