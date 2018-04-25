#pragma once

#include <atomic>
#include <mutex>
#include <condition_variable>


/**
 * Base class for messages passed between client and server threads.
 * Receivers of message will typically examine the "type", and down-cast
 * based on that.
 */
class ThreadMessage
{
public:
    enum class Type { EXIT };
    ThreadMessage(Type t) : type(t)
    {
        ++_dbgCount;
    }
    virtual ~ThreadMessage()
    {
        --_dbgCount;
    }

    const Type type;
    static std::atomic<int> _dbgCount;
};

/**
 * ThreadServer and ThreadClient do not refer to each other directly.
 * Instead, they both maintain pointers to ThreadSharedState.
 * All communication between thread goes through here.
 */
class ThreadSharedState
{
public:
    ThreadSharedState()
    {
        ++_dbgCount;
        serverRunning.store(false);
        mailbox.store(nullptr);
    }
    ~ThreadSharedState()
    {
        --_dbgCount;
    }
    std::atomic<bool> serverRunning;
    static std::atomic<int> _dbgCount;

    /**
     * if return false, message not sent.
     * otherwise message send, and msg may be reused.
     */
    bool trySendMessage(const ThreadMessage* msg);

    /**
     * returned message is a pointer to a message that we "own"
     * temporarily (sender wont modify it
     */
    const ThreadMessage* waitForMessage();
private:

    /** The message in the mailbox.
     * This is an object by whoever created it. Ownership of message
     * is not passed.
     *
     * Only the client writes to the mailbox. 
     */
    std::atomic<const ThreadMessage*> mailbox;
    std::mutex mailboxMutex;
    std::condition_variable mailboxCondition;
};