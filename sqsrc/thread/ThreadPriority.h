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

    static void restore();
private:

    static bool boostNormalPthread();
    static bool boostRealtimePthread();
    static void restorePthread();
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

inline void ThreadPriority::restore()
{
    restorePthread();
}

inline void ThreadPriority::restorePthread()
{
    const pthread_t threadHandle = pthread_self();
    struct sched_param params;
    params.sched_priority = 0;      // Note that on mac, this is not the default.
                                    // fix this.
    const int newPolicy = SCHED_OTHER;
    int x = pthread_setschedparam(threadHandle, newPolicy, &params);
    if (x != 0) {
        printf("failed to set reset sched %d\n", x);
    }
}

inline bool ThreadPriority::boostNormalPthread()
{
    struct sched_param params;
    const pthread_t threadHandle = pthread_self();
    int initPolicy = -10;
    pthread_getschedparam(threadHandle, &initPolicy, &params);
    printf("in boost, policy was %d, pri was %d otherp=%d\n", initPolicy, params.sched_priority, SCHED_OTHER);

    const int initPriority = params.sched_priority;
   
    const int newPolicy = SCHED_OTHER;
    const int maxPriority = sched_get_priority_max(newPolicy); 
#if 1
    {
        printf("for OTHER, pri range = %d,%d\n",
            sched_get_priority_min(newPolicy),
            sched_get_priority_max(newPolicy));
    }
#endif

    if (maxPriority <= initPriority) {
        // Linux seems to only offer one priority for SCHED_OTHER;
        printf("SCHED_OTHER only supports priority %d\n", maxPriority);
        return false;
    }

    params.sched_priority = maxPriority;
    int x = pthread_setschedparam(threadHandle, newPolicy, &params);
    if (x != 0) {
        printf("failed to set pri %d for SCHED_OTHER. error= %d\n", maxPriority, x);
    }
    return x == 0;
}

inline bool ThreadPriority::boostRealtimePthread()
{
    struct sched_param params;
    const pthread_t threadHandle = pthread_self();
    const int newPolicy = SCHED_RR;
    const int maxPriority = sched_get_priority_max(newPolicy);
    const int minPriority = sched_get_priority_min(newPolicy);


#if 0
    if ((maxPriority <= 0) || (minPriority < 0)) {
        printf("algorithm doesn't work with rt %d, %d\n", minPriority, maxPriority);
          return false;
    }
#endif

    // use the mean of min and max. These should all be higher than "non realtime" 
    // on mac the mean was not as good as elevating as other, to let's try max/
    //const int newPriority = (maxPriority + minPriority) / 2;
    const int newPriority = maxPriority; 
    
    printf("realtime min = %d max = %d will use %d\n", minPriority, maxPriority, newPriority);
    params.sched_priority = newPriority;
    int x = pthread_setschedparam(threadHandle, newPolicy, &params);
    if (x != 0) {
        printf("failed to set pri %d for SCHED_RR. error= %d\n", newPriority, x);
    }
    return x == 0;
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
inline void ThreadPriority::restore()
{

}
#endif
