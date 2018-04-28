
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
    //printf("noise client dtor\n"); fflush(stdout);
    std::unique_ptr<ThreadMessage> msg(new ThreadMessage(ThreadMessage::Type::EXIT));

    for (bool busy = true; busy; ) {
        if (sharedState->client_trySendMessage(msg.get())) {
            busy = false;
        }
    }
    //printf("noise client dtor2\n"); fflush(stdout);
    //const bool running = sharedState->serverRunning;
    //printf("noise client dtor will wait server running=%d\n", running ); fflush(stdout);
    while (sharedState->serverRunning) {
        static bool did = false;
        if (!did) {
            //printf("noise client dtor waiting\n"); fflush(stdout);
            did = true;
        }
    }
   // printf("noise client dtor exiting\n"); fflush(stdout);
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