
#include <assert.h>
#include "NoiseServer.h"
#include "NoiseSharedState.h"

NoiseServer::NoiseServer(std::shared_ptr<NoiseSharedState> state) : 
    sharedState(state)
{
}

NoiseServer::~NoiseServer()
{
}

void NoiseServer::start()
{
   // printf("starting thread\n"); fflush(stdout);
    auto startupFunc = [this]() {
        this->threadFunction();
    };
    std::unique_ptr<std::thread> th(new std::thread(startupFunc));
    thread = std::move(th);
}

void NoiseServer::threadFunction()
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

bool NoiseServer::procMessage(const NoiseMessage& msg)
{
    bool exit = false;
    switch (msg.type) {
        case NoiseMessage::Type::EXIT:
            exit = true;
            break;
        default:
            assert(false);
    }
    return exit;
}
