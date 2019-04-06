#pragma once


#include "Seq.h"
#include "MidiSequencer.h"
#include "seq/SequencerSerializer.h"

class SequencerWidget;
#include "WidgetComposite.h"
#include "engine/Module.hpp"

#include <atomic>

struct SequencerModule : rack::engine::Module
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

    std::atomic<bool> runStopRequested;
};