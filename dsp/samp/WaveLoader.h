#pragma once

#include <assert.h>
#include <memory>
#include <string>
#include <vector>

#include "FilePath.h"

class WaveLoader {
public:
    enum class Tests {
        None,
        DCOneSec,  // let's make one fake wave file that has one second of DC
        DCTenSec
    };

    class WaveInfo {
    public:
        WaveInfo(const FilePath& fileName);
        WaveInfo(Tests);                    // Special test only constructor
        ~WaveInfo();
        bool load(std::string& errorMsg);

        bool valid = false;
        float* data = nullptr;
        unsigned int numChannels = 0;
        unsigned int sampleRate = 0;
        uint64_t totalFrameCount = 0;
        const FilePath fileName;

        void validate() {
            assert(numChannels == 1);
            for (uint64_t i = 0; i < totalFrameCount; ++i) {
                const float d = data[i];
                assert(d <= 1);
                assert(d >= -1);
            }
        }

    private:
        // static float* convertToMono(float* data, uint64_t frames, int channels);
        void convertToMono();
    };
    using WaveInfoPtr = std::shared_ptr<WaveInfo>;

    /** Sample files are added one at a time until "all"
     * are loaded.
     */
    void addNextSample(const FilePath& fileName);

    /**
     * load() is called one - after all the samples have been added.
     * load will load all of them.
     * will return true is they all load.
     */

    enum class LoaderState {
        Done,
        Error,
        Progress
    };
    LoaderState load2();
    float getProgressPercent() const;

    /**
     * Index is one based. 
     */
    WaveInfoPtr getInfo(int index) const;
    std::string lastError;


    void _setTestMode(Tests);
private:
    Tests _testMode = Tests::None;

    std::vector<FilePath> filesToLoad;
    std::vector<WaveInfoPtr> finalInfo;
    void clear();
    bool didLoad = false;
    void validate();
    int curLoadIndex = -1;
};

using WaveLoaderPtr = std::shared_ptr<WaveLoader>;