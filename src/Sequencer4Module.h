#pragma once

#include "Seq4.h"
#include "WidgetComposite.h"
using Module =  ::rack::engine::Module;

#include <atomic>

/**
 */
class Sequencer4Module : public Module
{
public:
    Sequencer4Module();
    /**
     *
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;

    //std::shared_ptr<Comp> blank;
    std::shared_ptr<Seq4<WidgetComposite>> seq4Comp;

    void toggleRunStop()
    {
        runStopRequested = true;
    }

private:
    std::atomic<bool> runStopRequested;

};

