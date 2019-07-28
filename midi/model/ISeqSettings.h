#pragma once

#include <memory>
namespace rack {
    namespace widget {
        struct Widget;
    }
}

class ISeqSettings
{
public:
    virtual ~ISeqSettings()
    {
    }
    virtual void invokeUI(rack::widget::Widget* parent) = 0;
    virtual float getQuarterNotesInGrid() = 0;
    virtual bool snapToGrid() = 0;

    /**
     * if snap to grid,will quantize the passed value to the current grid.
     * otherwise does nothing.
     * will not quantize smaller than a grid.
     */
    virtual float quantize(float x, bool allowZero) = 0;
};

using ISeqSettingsPtr = std::shared_ptr<ISeqSettings>;
