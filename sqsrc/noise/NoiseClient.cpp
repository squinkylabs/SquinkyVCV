
#include "NoiseClient.h"
#include "NoiseServer.h"
#include "NoiseSharedState.h"


NoiseClient::NoiseClient(std::shared_ptr<NoiseSharedState> state,
    std::unique_ptr<NoiseServer> server) : sharedState(state), _server(std::move(server))
{
    printf("noise client starting server\n"); fflush(stdout);
    _server->start();
    while (!sharedState->serverRunning) {
        printf("noise client waiting server\n"); fflush(stdout);
    }
    printf("noise client started\n"); fflush(stdout);
}

NoiseClient::~NoiseClient()
{
    printf("noise client dtor\n"); fflush(stdout);
    std::unique_ptr<NoiseMessage> msg(new NoiseMessage(NoiseMessage::Type::EXIT));

    for (bool busy = true; busy; ) {
        if (sharedState->trySendMessage(msg.get())) {
            busy = false;
        }
    }
    const bool running = sharedState->serverRunning;
    printf("noise client dtor will wait server running=%d\n", running ); fflush(stdout);
    while (sharedState->serverRunning) {
        static bool did = false;
        if (!did) {
            printf("noise client dtor waiting\n"); fflush(stdout);
            did = true;
        }
    }
    printf("noise client dtor exiting\n"); fflush(stdout);
}