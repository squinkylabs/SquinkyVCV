
#include <assert.h>
#include "NoiseSharedState.h"

std::atomic<int> NoiseSharedState::_dbgCount = 0;


std::shared_ptr<NoiseMessage>  NoiseSharedState::waitForMessage()
{
    std::unique_lock<std::mutex> guard(mailboxMutex);        // grab the mutex that protects condition
    mailboxCondition.wait(guard);                            // wait for client to send a message
    if (!mailbox) {
        return std::shared_ptr<NoiseMessage>();             // spurious wakeup - return nothing
    } else {
        // make a copy of the one in the mailbox. We can own this copy
        std::shared_ptr<NoiseMessage> theMessage = std::make_shared<NoiseMessage>(*mailbox);
        mailbox = nullptr;
        return theMessage;
    }
}

bool NoiseSharedState::trySendMessage(const NoiseMessage* msg)
{
    // If the client tries to send a message before the previous one is read, the
    // call will fail and the client must try again.
    if (mailbox) {
        return false;
    }
    std::unique_lock<std::mutex> guard(mailboxMutex, std::defer_lock);

    // We must use a try_lock here, as calling regular lock() could cause a priority inversion.
    bool didLock = guard.try_lock();
    if (didLock) {
        assert(!mailbox);               // if there is still a message there we are out of sync
        mailbox = msg;
        mailboxCondition.notify_one();
    }
    return didLock;
}
