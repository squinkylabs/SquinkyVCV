#pragma once

#include <atomic>
#include <mutex>
#include <condition_variable>


class NoiseMessage
{
public:
    enum class Type { EXIT };
    NoiseMessage(Type t) : type(t)
    {
    }

    const Type type;
};

class NoiseSharedState
{
public:
    NoiseSharedState()
    {
        ++_dbgCount;
    }
    ~NoiseSharedState()
    {
        --_dbgCount;
    }
    std::atomic<bool> serverRunning;
    static std::atomic<int> _dbgCount;

    /**
     * if return false, message not sent.
     * otherwise message send, and msg may be reused.
     */
    bool trySendMessage(const NoiseMessage* msg);

    /**
     * returned message is a copy we own
     */
    std::shared_ptr<NoiseMessage> waitForMessage();
private:
    /** The message in the mailbox.
     * This is an object that is owned by the client. We may not
     * modify it or delete it.
     *
     * Only the client writes to the mailbox. 
     */
    std::atomic<const NoiseMessage*> mailbox=nullptr;
    std::mutex mailboxMutex;
    std::condition_variable mailboxCondition;
};