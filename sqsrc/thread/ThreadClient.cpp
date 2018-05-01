
#include "ThreadClient.h"
#include "ThreadServer.h"
#include "ThreadSharedState.h"

#include <assert.h>


ThreadClient::ThreadClient(std::shared_ptr<ThreadSharedState> state,
    std::unique_ptr<ThreadServer> server) : sharedState(state), _server(std::move(server))
{
    assert(!sharedState->serverRunning);
    //printf("noise client starting server\n"); fflush(stdout);
    _server->start();
    while (!sharedState->serverRunning) {
        //printf("noise client waiting server\n"); fflush(stdout);
    }

    //printf("noise client started\n"); fflush(stdout);
}

ThreadClient::~ThreadClient()
{
    printf("Thread client dtor\n"); fflush(stdout);

    sharedState->client_askServerToStop();
    sharedState->serverStopRequested.store(true);               // ask server to stop

    printf("thread client dtor2\n"); fflush(stdout);
    const bool running = sharedState->serverRunning;
    printf("thread client dtor will wait server running=%d\n", running ); fflush(stdout);
    while (sharedState->serverRunning) {
        static bool did = false;
        if (!did) {
            printf("thread client dtor waiting\n"); fflush(stdout);
            did = true;
        }
    }
    printf("thread client dtor exiting\n"); fflush(stdout);
}

ThreadMessage * ThreadClient::getMessage()
{
    //return nullptr;

    return sharedState->client_pollMessage();
}


bool ThreadClient::sendMessage(ThreadMessage * message)
{
    return sharedState->client_trySendMessage(message);
}