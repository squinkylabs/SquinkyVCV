
#include <assert.h>
#include "ThreadSharedState.h"

std::atomic<int> ThreadSharedState::_dbgCount;
std::atomic<int> ThreadMessage::_dbgCount;

#include <iostream>
#include <chrono>
#include <thread>

const ThreadMessage*  ThreadSharedState::waitForMessage()
{
   // printf("wait\n"); fflush(stdout);

    std::unique_lock<std::mutex> guard(mailboxMutex);           // grab the mutex that protects condition
    const ThreadMessage* returnMessage = nullptr;
    while (!returnMessage) {
        returnMessage = mailboxClient2Server.load();            // don't wait on condition if we already have it.
        if (!returnMessage) {
            mailboxCondition.wait(guard);                       // wait for client to send a message
            returnMessage = mailboxClient2Server.load();
        }
    }
    // This simple method of cloning won't work for message with data
    return returnMessage;
}

// signal in lock
bool ThreadSharedState::trySendMessage(const ThreadMessage* msg)
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


