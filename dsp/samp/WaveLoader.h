#pragma once

#include <memory>
#include <string>
#include <vector>

#include "FilePath.h"

class WaveLoader {
public:
    class WaveInfo {
    public:
        WaveInfo(const FilePath& fileName);
        ~WaveInfo();
        bool load(std::string& errorMsg);

        bool valid = false;
        float* data = nullptr;
        unsigned int numChannels = 0;
        unsigned int sampleRate = 0;
        uint64_t totalFrameCount = 0;
        const FilePath fileName;

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
    bool load();

    /**
     * Index is one based. 
     */
    WaveInfoPtr getInfo(int index) const;

#if 0 // use FilePath instead
    static char nativeSeparator();
    static char foreignSeparator();
    static void makeAllSeparatorsNative(std::string& s);
#endif

    std::string lastError;
private:

    std::vector<FilePath> filesToLoad;
    std::vector<WaveInfoPtr> finalInfo;
    void clear();
    bool didLoad = false;
};

using WaveLoaderPtr = std::shared_ptr<WaveLoader>;