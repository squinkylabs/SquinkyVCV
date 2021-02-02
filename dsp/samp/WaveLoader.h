#pragma once

#include <memory>
#include <string>
#include <vector>

class WaveLoader {
public:
    class WaveInfo {
    public:
        WaveInfo(const std::string& fileName);
        ~WaveInfo();
        bool load();

        bool valid = false;
        float* data = nullptr;
        unsigned int numChannels = 0;
        unsigned int sampleRate = 0;
        uint64_t totalFrameCount = 0;
        const std::string fileName;

    private:
        // static float* convertToMono(float* data, uint64_t frames, int channels);
        void convertToMono();
    };
    using WaveInfoPtr = std::shared_ptr<WaveInfo>;

    /** Sample files are added one at a time until "all"
     * are loaded.
     */
    void addNextSample(const std::string& fileName);

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

    static char nativeSeparator();
    static char foreignSeparator();
    static void makeAllSeparatorsNative(std::string& s);

private:
    std::vector<std::string> filesToLoad;
    std::vector<WaveInfoPtr> finalInfo;
    void clear();
    bool didLoad = false;
};

using WaveLoaderPtr = std::shared_ptr<WaveLoader>;