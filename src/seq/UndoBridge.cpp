
#include "SqCommand.h"
#include "UndoRedoStack.h"

// VCV includes
#include "app.hpp"
#include "history.hpp"

// std C
#include <assert.h>

#ifdef __USE_VCV_UNDO

class SeqAction : public rack::history::ModuleAction
{
public: 
    SeqAction(const std::string& name, std::shared_ptr<SqCommand> command, int moduleId) {
        wrappedCommand = command; 
        this->name = wrappedCommand->name;
        this->moduleId = moduleId;
    }
    void undo() override
    {
        wrappedCommand->undo();
    }
    void redo() override
    {
        wrappedCommand->execute();
    }

private:
    std::shared_ptr<SqCommand> wrappedCommand;
};

void UndoRedoStack::setModuleId(int id) 
{
    this->moduleId = id;
}

void UndoRedoStack::execute(std::shared_ptr<SqCommand> cmd)
{
    cmd->execute();
    auto action = new SeqAction("unknown", cmd, moduleId);

    APP->history->push(action);
}

#endif