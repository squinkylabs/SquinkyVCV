#pragma once

#include "MidiSelectionModel.h"
/**
 * Suppresses note audition
 */
class AuditionLocker
{
public:
    AuditionLocker(MidiSelectionModelPtr m) : model(m)
    {
        wasSuppressed = model->isAuditionSuppressed();
        model->setAuditionSuppressed(true);
    }
    ~AuditionLocker()
    {
        model->setAuditionSuppressed(wasSuppressed);
    }
private:
    bool wasSuppressed = false;
    MidiSelectionModelPtr model;
};