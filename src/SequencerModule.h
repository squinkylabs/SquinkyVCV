#pragma once


#include "Seq.h"
#include "MidiSequencer.h"
#include "seq/SequencerSerializer.h"

class SequencerWidget;
#include "WidgetComposite.h"

#ifdef __V1
    #include "engine/Module.hpp"
    using Module =  rack::engine::Module;
#else
    #include "Engine.hpp"
    using Module =  rack::Module;
#endif

#include <atomic>

struct SequencerModule : Module
{
    SequencerModule();
    std::shared_ptr<Seq<WidgetComposite>> seqComp;

    MidiSequencerPtr sequencer;
    SequencerWidget* widget = nullptr;


    void step() override
    {
    #ifdef __V1
        sequencer->undo->setModuleId(this->id);
    #endif
        if (runStopRequested) {
            seqComp->toggleRunStop();
            runStopRequested = false;
        }
        seqComp->step();
    }
    void onReset() override;

    void stop()
    {
        seqComp->stop();
    }

    float getPlayPosition()
    {
        return seqComp->getPlayPosition();
    }

    MidiSequencerPtr getSeq() {
        return sequencer;
    }

    void toggleRunStop()
    {
        runStopRequested = true;
    }

    bool isRunning()
    {
        return seqComp->isRunning();
    }

#ifndef __V1
    json_t *toJson() override
    {
        assert(sequencer);
        return SequencerSerializer::toJson(sequencer);
    }
    void fromJson(json_t* data) override;
#else
    virtual json_t *dataToJson() override
    {
        assert(sequencer);
        return SequencerSerializer::toJson(sequencer);
    }
    virtual void dataFromJson(json_t *root) override;
#endif
private:
    void setNewSeq(MidiSequencerPtr);


    std::atomic<bool> runStopRequested;
};