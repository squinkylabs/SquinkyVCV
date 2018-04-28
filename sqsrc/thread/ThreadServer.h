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
    /**
     * Derived thread servers must override this to get messages.
     * Message is not const, as server is allowed to modify it and 
     * send it back.
     */
    virtual void handleMessage(ThreadMessage*);

    /**
     * Utility for sending replies back to the  client.
     * Will causes an error if there is a message in the mailbox already.
     */
    void sendMessageToClient(ThreadMessage*);

private:
    std::shared_ptr<ThreadSharedState> sharedState;
    std::unique_ptr<std::thread> thread;

    void threadFunction();

    /**
     * returns true to exit thread.
     * TODO: does it really?
     */
    bool procMessage(ThreadMessage*);
};