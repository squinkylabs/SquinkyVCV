#pragma once

class ThreadPriority
{
public:
    /**
     * Attempts to boost priority of current thread, but not all
     * the way to a "realtime" priority. In genera, should not require
     * admin rights.
     */
    static bool boostNormal();

    /**
    * Attempts to boost priority of current thread all the way to
    * a "realtime" priority. Will require admin rights
    */
    static bool boostRealtime();
private:

    static bool boostNormalPthread();
    static bool boostRealtimePthread();
};

// Inside Visual Studio test we don't try to link in PThreads, 
// So they can't be used here. But they will work on all command line
// test builds, including Windows.
#if !defined(_MSC_VER)
inline bool ThreadPriority::boostNormal()
{
    return boostNormalPthread();
}

inline bool ThreadPriority::boostRealtime()
{
    return boostRealtimePthread();
}


inline bool ThreadPriority::boostNormalPthread()
{
    const pthread_t threadHandle = pthread_self();
    int policy = SCHED_OTHER;
}
#else
inline bool ThreadPriority::boostNormal()
{
    return true;
}

inline bool ThreadPriority::boostRealtime()
{
    return true;
}
#endif

#if 0
// temporary
void tryThread()
{
    pthread_t threadHandle = pthread_self();

    struct sched_param params;
    int policy = 55;
    pthread_getschedparam(threadHandle, &policy, &params);
    printf("default was policy %d priority %d\n", policy, params.sched_priority);

    printf("FIFO=%d OTHER=%d\n", SCHED_FIFO, SCHED_OTHER);
    // first, let's go for max. only works if root
    policy = SCHED_RR;
    int maxFifo = sched_get_priority_max(policy);
    int y = sched_get_priority_min(policy);
    printf("for rr, max=%d min=%d\n", maxFifo, y);

    params.sched_priority = maxFifo;
    int x = pthread_setschedparam(threadHandle, policy, &params);
    printf("set realtime ret %d. 0 is success. pri=%d\n", x, maxFifo);
    if (x != 0) {
        int policy = SCHED_OTHER;
        int maxOther = sched_get_priority_max(policy);
        y = sched_get_priority_min(policy);
        printf("for sched_other, max=%d min=%d\n", maxOther, y);
        params.sched_priority = maxOther;
        x = pthread_setschedparam(threadHandle, policy, &params);
        printf("set realtime ret %d. 0 is success. pri=%d\n", x, maxOther);

    }
    printf(" ESRCH: %d, EINVAL %d, EPERM %d ENOTSUP %d\n", ESRCH, EINVAL, EPERM, ENOTSUP);
}
#endif
