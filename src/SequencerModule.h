#pragma once


#include "Seq.h"
#include "MidiSequencer.h"
#include "seq/SequencerSerializer.h"

class SequencerWidget;
#include "WidgetComposite.h"

#ifdef __V1x
    #include "engine/Module.hpp"
    using Module =  ::rack::engine::Module;
#else
    #include "engine.hpp"
    using Module =  ::rack::Module;
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
        sequencer->undo->setModuleId(this->id);
        if (runStopRequested) {
            seqComp->toggleRunStop();
            runStopRequested = false;
        }
        seqComp->step();
    }
    void onReset() override;

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

    void onSampleRateChange() override {
        seqComp->onSampleRateChange();
    }

    int getAuditionParamId()
    {
        return Seq<WidgetComposite>::AUDITION_PARAM;
    }

    virtual json_t *dataToJson() override
    {
        assert(sequencer);
        return SequencerSerializer::toJson(sequencer);
    }
    virtual void dataFromJson(json_t *root) override;

    // May be called from UI thread
    void postNewSong(MidiSongPtr s);
private:
    void setNewSeq(MidiSequencerPtr);


    std::atomic<bool> runStopRequested;
};