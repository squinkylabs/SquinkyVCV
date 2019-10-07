
#include "SqCommand.h"
#include "UndoRedoStack.h"

// std C
#include <assert.h>

#if defined(__USE_VCV_UNDO) && defined(_SEQ)

// VCV includes
#include "app.hpp"
#include "history.hpp"

#include "../SequencerModule.h"

class SeqAction : public rack::history::ModuleAction
{
public:
    SeqAction(const std::string& _name, std::shared_ptr<SqCommand> command, int moduleId)
    {
        wrappedCommand = command;
        this->name = "Seq++: " + wrappedCommand->name;
        this->moduleId = moduleId;
    }
    void undo() override
    {
        MidiSequencerPtr seq = getSeq();
        if (seq) {
            wrappedCommand->undo(seq);
        }
    }
    void redo() override
    {
        MidiSequencerPtr seq = getSeq();
        if (seq) {
            wrappedCommand->execute(seq);
        }
    }

private:
    std::shared_ptr<SqCommand> wrappedCommand;
    MidiSequencerPtr getSeq()
    {
        MidiSequencerPtr ret;
        SequencerModule* module = dynamic_cast<SequencerModule *>(::rack::APP->engine->getModule(moduleId));
        if (!module) {
            fprintf(stderr, "error getting module in undo\n");
            return ret;
        }
        ret = module->sequencer;
        if (!ret) {
            fprintf(stderr, "error getting sequencer in undo\n");
        }
        return ret;
    }
};

void UndoRedoStack::setModuleId(int id)
{
    this->moduleId = id;
}

void UndoRedoStack::execute(MidiSequencerPtr seq, std::shared_ptr<SqCommand> cmd)
{
    assert(seq);
    cmd->execute(seq);
    auto action = new SeqAction("unknown", cmd, moduleId);

    ::rack::APP->history->push(action);
}

#endif


#if defined(__USE_VCV_UNDO) && !defined(_SEQ)

void UndoRedoStack::setModuleId(int id)
{
    ;
}

void UndoRedoStack::execute(MidiSequencerPtr seq, std::shared_ptr<SqCommand> cmd)
{

}
#endif
