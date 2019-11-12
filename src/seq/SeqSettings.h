#pragma once


#include "ISeqSettings.h"

#include "rack.hpp"

#include <stdio.h>

class SequencerModule;

class SeqSettings : public ISeqSettings
{
public:
    friend class GridMenuItem;
    friend class ArticulationMenuItem;
    friend class GridItem;
    friend class ArticItem;
    friend class SequencerSerializer;

    SeqSettings(SequencerModule*);
    ~SeqSettings() override;

    /**
     * returns the top level menu it builds
     */
    ::rack::ui::Menu* invokeUI (::rack::widget::Widget* parent) override;

    /**
     * Grid related settings
     */
    float getQuarterNotesInGrid() override;
    bool snapToGrid() override;
    bool snapDurationToGrid() override;
    float quantize(float, bool allowZero) override;
    float quantizeAlways(float, bool allowZero) override;

    float articulation() override;

    std::string getMidiFilePath() override;
    void setMidiFilePath(const std::string&) override;

    virtual std::pair<int, Scale::Scales> getKeysig() override;
    void setKeysig(int root, Scale::Scales mode) override;
private:
    SequencerModule* const module;

    enum class Grids
    {
        quarter,
        eighth,
        sixteenth
    };

    enum class Artics
    {
        tenPercent,
        twentyPercent,
        fiftyPercent,
        eightyFivePercent,
        oneHundredPercent,
        legato
    };

    std::string getGridString() const;
    std::string getArticString() const;

    static std::vector<std::string> getGridLabels();
    static Grids gridFromString(const std::string& s);
    static std::vector<std::string> getArticulationLabels();
    static Artics articFromString(const std::string& s);

    Grids curGrid = Grids::sixteenth;
    Artics curArtic = Artics::eightyFivePercent;

    bool snapEnabled = true;
    bool snapDurationEnabled = false;

    std::string midiFilePath;

    int keysigRoot = 0;
    Scale::Scales keysigMode = Scale::Scales::Minor;

    static float grid2Time(Grids);
    static float artic2Number(Artics);
    ::rack::ui::MenuItem* makeSnapItem();
    ::rack::ui::MenuItem* makeLoopItem(SequencerModule* module);
    ::rack::ui::MenuItem* makeSnapDurationItem();
    ::rack::ui::MenuItem* makeAuditionItem(SequencerModule*);
    ::rack::ui::MenuItem* makeNoteCommand(SequencerModule*);
    ::rack::ui::MenuItem* makeEndCommand(SequencerModule*);
};