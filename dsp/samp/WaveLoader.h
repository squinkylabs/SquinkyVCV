#pragma once

#include <assert.h>
#include <memory>
#include <string>
#include <vector>

#include "FilePath.h"

// Abstract interface for audio files
class WaveInfoInterface {
public:
    virtual ~WaveInfoInterface() = default;
    virtual unsigned int getSampleRate() = 0;
    virtual uint64_t getTotalFrameCount() = 0;
    virtual bool isValid() const = 0;
    virtual const float* getData() = 0;
    virtual bool load(std::string& errorMsg) = 0;
    virtual std::string getFileName() = 0;
};

class WaveLoader {
public:
    enum class Tests {
        None,
        DCOneSec,  // let's make one fake wave file that has one second of DC
        DCTenSec,
        RampOneSec
    };
    using WaveInfoPtr = std::shared_ptr<WaveInfoInterface>;

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

    // load up all the registered files
    
    LoaderState loadNextFile();
    float getProgressPercent() const;

    /**
     * Index is one based. 
     */
    WaveInfoPtr getInfo(int index) const;
    std::string lastError;


    void _setTestMode(Tests);
private:
   // Tests _testMode = Tests::None;

    std::vector<FilePath> filesToLoad;
    std::vector<WaveInfoPtr> finalInfo;
    static WaveInfoPtr loaderFactory(const FilePath& file);
    void clear();
    bool didLoad = false;
    void validate();
    int curLoadIndex = -1;
};

using WaveLoaderPtr = std::shared_ptr<WaveLoader>;

#if 0
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
   #endif