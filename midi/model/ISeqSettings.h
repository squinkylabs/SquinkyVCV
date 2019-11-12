#pragma once

#include "Scale.h"

#include <string>
#include <memory>
#include <utility>


namespace rack {
    namespace widget {
        struct Widget;
    }
    namespace ui {
        struct Menu;
    }
}

class ISeqSettings
{
public:
    virtual ~ISeqSettings()
    {
    }
    virtual ::rack::ui::Menu* invokeUI (::rack::widget::Widget* parent) = 0;

    /**
     * Grid things
     */
    virtual float getQuarterNotesInGrid() = 0;
    virtual bool snapToGrid() = 0;
    virtual bool snapDurationToGrid() = 0;

    /**
     * If snap to grid,will quantize the passed value to the current grid.
     * otherwise does nothing.
     * will not quantize smaller than a grid.
     */
    virtual float quantize(float x, bool allowZero) = 0;

    virtual float quantizeAlways(float x, bool allowZer0) = 0;

    /**
     * articulation of inserted notes.
     * 1 = 100%
     */
    virtual float articulation() = 0;

    /**
     * minor hack: let's keep the midi file path here, too.
     * (it should really be in global settings).
     */
    virtual std::string getMidiFilePath() = 0;
    virtual void setMidiFilePath(const std::string&) = 0;

    virtual std::pair<int, Scale::Scales> getKeysig() = 0;
    virtual void setKeysig(int root, Scale::Scales mode) = 0;
};

using ISeqSettingsPtr = std::shared_ptr<ISeqSettings>;
