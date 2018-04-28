
#include <assert.h>
#include "ThreadSharedState.h"

std::atomic<int> ThreadSharedState::_dbgCount;
std::atomic<int> ThreadMessage::_dbgCount;

#include <iostream>
#include <chrono>
#include <thread>

ThreadMessage*  ThreadSharedState::server_waitForMessage()
{
   // printf("wait\n"); fflush(stdout);

    std::unique_lock<std::mutex> guard(mailboxMutex);           // grab the mutex that protects condition
    ThreadMessage* returnMessage = nullptr;
    while (!returnMessage) {
        returnMessage = mailboxClient2Server.load();            // don't wait on condition if we already have it.
        if (!returnMessage) {
            mailboxCondition.wait(guard);                       // wait for client to send a message
            returnMessage = mailboxClient2Server.load();
        }
    }
    mailboxClient2Server.store(nullptr);                    // remove the message from the mailbox
                                                            // (should we lock here?) (no, we will have lock)


    return returnMessage;
}

ThreadMessage* ThreadSharedState::client_pollMessage()
{
    ThreadMessage* msg = nullptr;

    // grab lock
    std::unique_lock<std::mutex> guard(mailboxMutex);
    msg = mailboxServer2Client.load();
    if (msg) {
        mailboxServer2Client.store(nullptr);
    }
   // printf("client poss message ret %p\n", msg);
    return msg;
}

// signal in lock
bool ThreadSharedState::client_trySendMessage(ThreadMessage* msg)
{
    //printf("snd\n"); fflush(stdout);

    assert(serverRunning.load());
    // If the client tries to send a message before the previous one is read, the
    // call will fail and the client must try again.
    if (mailboxClient2Server.load()) {
        return false;
    }

    // Write to mailbox (condition) in lock

    std::unique_lock<std::mutex> guard(mailboxMutex, std::defer_lock);
    // We must use a try_lock here, as calling regular lock() could cause a priority inversion.
    bool didLock = guard.try_lock();
    if (!didLock) {
        return false;
    }
    assert(guard.owns_lock());

    assert(!mailboxClient2Server.load());               // if there is still a message there we are out of sync
    mailboxClient2Server.store(msg);

    mailboxCondition.notify_all();
    //printf("sx\n"); fflush(stdout);
    return true;
}

void ThreadSharedState::server_sendMessage(ThreadMessage* msg)
{
    std::unique_lock<std::mutex> guard(mailboxMutex);
    assert(mailboxServer2Client.load() == nullptr);
    mailboxServer2Client.store(msg);  
}


