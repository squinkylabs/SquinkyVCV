
#include <assert.h>
#include "ThreadServer.h"
#include "ThreadSharedState.h"

ThreadServer::ThreadServer(std::shared_ptr<ThreadSharedState> state) : 
    sharedState(state)
{
}

ThreadServer::~ThreadServer()
{
}

void ThreadServer::start()
{
   // printf("starting thread\n"); fflush(stdout);
    auto startupFunc = [this]() {
        this->threadFunction();
    };
    std::unique_ptr<std::thread> th(new std::thread(startupFunc));
    thread = std::move(th);
}

void ThreadServer::threadFunction()
{
    sharedState->serverRunning = true;
    for (bool done = false; !done; ) {
        auto msg = sharedState->waitForMessage();
        assert(msg);
        done = procMessage(*msg);
    }

    //printf("noiseserer shut down\n"); fflush(stdout);
    thread->detach();
    sharedState->serverRunning = false;
}

bool ThreadServer::procMessage(const ThreadMessage& msg)
{
    bool exit = false;
    switch (msg.type) {
        case ThreadMessage::Type::EXIT:
            exit = true;
            break;
        default:
            handleMessage(msg);
    }
    return exit;
}

void ThreadServer::handleMessage(const ThreadMessage&)
{
    assert(false);          // derived must override.
}
