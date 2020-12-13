#pragma once

#include <string>
#include <vector>

class WaveLoader
{
public:
    class WaveInfo {
    public:
       WaveInfo(const std::string& fileName);
        ~WaveInfo();
        void load();

        bool valid = false;
        float* data = nullptr;
        unsigned int numChannels = 0;
        unsigned int sampleRate = 0;
        uint64_t totalFrameCount = 0;
        const std::string fileName;
    };

    void load(const std::string& fileName);
    const WaveInfo& getInfo(int index) const;
private:
    std::vector<WaveInfo> info;
    void clear();
};