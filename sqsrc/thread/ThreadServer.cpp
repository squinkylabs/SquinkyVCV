
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
        if (sharedState->serverStopRequested.load()) {
            done = true;
        } else {
            // if msg is null, stop was requested
            ThreadMessage* msg = sharedState->server_waitForMessageOrShutdown();
            if (msg) {
                procMessage(msg);
            }
        }
    }

    //printf("noiseserer shut down\n"); fflush(stdout);
    thread->detach();
    sharedState->serverRunning = false;
}

//TODO: get rid of this function
void ThreadServer::procMessage(ThreadMessage* msg)
{

    handleMessage(msg);
}

void ThreadServer::handleMessage(ThreadMessage* )
{
    assert(false);          // derived must override.
}

void ThreadServer::sendMessageToClient(ThreadMessage* msg)
{
    sharedState->server_sendMessage(msg);
}