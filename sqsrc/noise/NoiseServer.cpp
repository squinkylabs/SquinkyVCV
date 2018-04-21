
#include <assert.h>
#include "NoiseServer.h"
#include "NoiseSharedState.h"

NoiseServer::NoiseServer(std::shared_ptr<NoiseSharedState> state) : 
    sharedState(state)
{
    printf("noise server ctor\n"); fflush(stdout);
}

NoiseServer::~NoiseServer()
{
    printf("noise server dtorx %p\n", this); fflush(stdout);
}

void NoiseServer::start()
{
    printf("starting thread\n"); fflush(stdout);
    auto startupFunc = [this]() {
        this->threadFunction();
    };
    std::unique_ptr<std::thread> th(new std::thread(startupFunc));
    thread = std::move(th);
}

void NoiseServer::threadFunction()
{
    printf("in thread func\n"); fflush(stdout);
    sharedState->serverRunning = true;
    for (bool done = false; !done; ) {
        printf("thread func got a message\n"); fflush(stdout);
        auto msg = sharedState->waitForMessage();
        done = procMessage(*msg);
    }

    printf("thread func shutting down\n"); fflush(stdout);
    sharedState->serverRunning = false;
    printf("thread func about to detach\n"); fflush(stdout);
    thread->detach();
    printf("thread func detached\n"); fflush(stdout);
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
