#pragma once

#include <memory>
#include <thread>

class ThreadSharedState;
class ThreadMessage;

class ThreadServer
{
public:
    ThreadServer(std::shared_ptr<ThreadSharedState> state);
    virtual ~ThreadServer();
    void start();

    const ThreadServer& operator= (const ThreadServer&) = delete;
    ThreadServer(const ThreadServer&) = delete;
    static int _count;
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
     *
     * TODO: get rid of proc and handle, if possible
     */
    void procMessage(ThreadMessage*);
};