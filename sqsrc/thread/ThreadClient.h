#pragma once
#include <memory>

class ThreadSharedState;
class ThreadServer;

class ThreadClient
{
public:
    ThreadClient(std::shared_ptr<ThreadSharedState> state, std::unique_ptr<ThreadServer> server);
    ~ThreadClient();

    const ThreadClient& operator= (const ThreadClient&) = delete;
    ThreadClient(const ThreadClient&) = delete;

private:
    std::shared_ptr<ThreadSharedState> sharedState;
    std::unique_ptr<ThreadServer> _server;
};