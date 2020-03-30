#pragma once


#include "Seq.h"
#include "MidiSequencer.h"
#include "seq/SequencerSerializer.h"

class SequencerWidget;
#include "WidgetComposite.h"
using Module =  ::rack::engine::Module;

#include <atomic>

class SequencerModule : public Module
{
public:
    SequencerModule();
    std::shared_ptr<Seq<WidgetComposite>> seqComp;

    MidiSequencerPtr getSequencer() { return sequencer; }

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

    /**
     *  May be called from UI thread.
     *  @param s is the new song to load.
     *  @param path is the file folder it was loaded from.
     *  @param doUndo determines if we make a new undo event or not.
     */
    void postNewSong(MidiSongPtr s, const std::string& path, bool doUndo);
private:
    void setNewSeq(MidiSequencerPtr);
    std::atomic<bool> runStopRequested;
};