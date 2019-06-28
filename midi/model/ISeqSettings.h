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
};

using ISeqSettingsPtr = std::shared_ptr<ISeqSettings>;
