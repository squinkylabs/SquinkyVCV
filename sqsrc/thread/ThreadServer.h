#pragma once

#include <memory>
#include <thread>

class ThreadSharedState;
class ThreadMessage;

class ThreadServer
{
public:
    ThreadServer(std::shared_ptr<ThreadSharedState> state);
    ~ThreadServer();
    void start();

    const ThreadServer& operator= (const ThreadServer&) = delete;
    ThreadServer(const ThreadServer&) = delete;

protected:
    virtual void handleMessage(const ThreadMessage&);

private:
    std::shared_ptr<ThreadSharedState> sharedState;
    std::unique_ptr<std::thread> thread;

    void threadFunction();

    /**
     * returns true to exit thread
     */
    bool procMessage(const ThreadMessage&);
};