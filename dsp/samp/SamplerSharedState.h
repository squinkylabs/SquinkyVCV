#pragma once

#include <atomic>
#include <memory>

#include "SqLog.h"

/**
 * These are variables that are shared by three threads:
 *      The VCV audio thread
 *      The VCV UI thread
 *      our worker thread
 * 
 * all functions are thread-safe when used
 * correctly.
 * 
 * Any function that the audio thread might call will be
 * non-blocking.
 */

class SamplerSharedState {
public:
    ~SamplerSharedState() {
        SQINFO("dtor of SamplerSharedState");
    }
    /** called from UI thread or worker thread
    */

    /* Ask for permission to change the sample data.
     * once permission is granted audio thread won't look at
     * sample data, so others are free to change it safely.
     */
    void uiw_requestSampleReload() {
        sampleReloadRequestGranted = false;
        sampleReloadRequested = true;
    }

    // Same as uiw_requestSampleReload, but wait for audio thread to grant it
    void uiw_requestAndWaitForSampleReload() {
        uiw_requestSampleReload();
        while (!sampleReloadRequestGranted) {
        }  // busy wait on atomic variable
    }

    float uiw_getProgressPercent() {
        return progressPercent;
    }

    void uiw_setLoadProgress(float pct) {
        progressPercent = pct;
    }

    // called from audio thread
    bool au_isSampleReloadRequested() const {
        return sampleReloadRequested;
    }
    void au_grantSampleReloadRequest() {
        sampleReloadRequested = false;
        sampleReloadRequestGranted = true;
    }

private:
    std::atomic<bool> sampleReloadRequested = {false};
    std::atomic<bool> sampleReloadRequestGranted = {false};
    std::atomic<float> progressPercent = {0};
};


using SamplerSharedStatePtr = std::shared_ptr<SamplerSharedState>;
