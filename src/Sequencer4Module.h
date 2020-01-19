#pragma once

#include "Seq4.h"
#include "WidgetComposite.h"

using Module =  ::rack::engine::Module;
class MidiSequencer4;

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

    std::shared_ptr<Seq4<WidgetComposite>> seq4Comp;

    void toggleRunStop()
    {
        runStopRequested = true;
    }
    MidiSong4Ptr getSong();

    json_t *dataToJson() override;
    void dataFromJson(json_t *data) override;
private:
    std::atomic<bool> runStopRequested;
    std::shared_ptr<MidiSequencer4> seq4;
};

