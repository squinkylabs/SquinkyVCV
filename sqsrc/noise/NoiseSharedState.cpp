
#include <assert.h>
#include "NoiseSharedState.h"

std::atomic<int> NoiseSharedState::_dbgCount = 0;


/*
std::shared_ptr<NoiseMessage> waitForMessage();
private:
    std::atomic<const NoiseMessage*> mailbox;
    std::mutex mailboxMutex;
    std::condition_variable mailboxCondition;
    */


std::shared_ptr<NoiseMessage>  NoiseSharedState::waitForMessage()
{
    std::unique_lock<std::mutex> guard(mailboxMutex);        // grab the mutex that protects condition
    mailboxCondition.wait(guard);                                // wait for client to send a message
    if (!mailbox) {
        return std::shared_ptr<NoiseMessage>();             // spurious wakeup - return nothing
    } else {
        return std::shared_ptr<NoiseMessage>(std::make_shared<NoiseMessage>(*mailbox));
    }

}

bool NoiseSharedState::trySendMessage(const NoiseMessage* msg)
{
    printf("try send message\n"); fflush(stdout);
    std::unique_lock<std::mutex> guard(mailboxMutex, std::defer_lock);
    bool didLock = guard.try_lock();
    if (didLock) {
        printf("sent message\n"); fflush(stdout);
        assert(!mailbox);               // if there is still a message there we are out of sync
        mailbox = msg;
        mailboxCondition.notify_one();
    }
    return didLock;

}
