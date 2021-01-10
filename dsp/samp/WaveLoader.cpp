
#include "SqLog.h"
#include "WaveLoader.h"

#include <assert.h>
#include <algorithm>

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

void WaveLoader::clear() {
    finalInfo.clear();
}

WaveLoader::WaveInfoPtr WaveLoader::getInfo(int index) const {
    assert(didLoad);
    if (index < 1 || index > int(finalInfo.size())) {
        return nullptr;
    }
    return finalInfo[index - 1];
}

WaveLoader::WaveInfo::WaveInfo(const std::string& path) : fileName(path) {
}

void WaveLoader::addNextSample(const std::string& fileName) {
    assert(!didLoad);
    printf("adding %s\n", fileName.c_str());

    auto x = fileName.find(foreignSeparator());
    assert(x == std::string::npos);

    filesToLoad.push_back(fileName);
}

void WaveLoader::load() {
    printf("waveLoader::load %lld\n", filesToLoad.size());
    assert(!didLoad);
    didLoad = true;
    for (std::string& file : filesToLoad) {
        WaveInfoPtr waveInfo = std::make_shared<WaveInfo>(file);
        waveInfo->load();

        finalInfo.push_back(waveInfo);
        //printf("adding one\n");
    }
}

void WaveLoader::WaveInfo::load() {
    // printf("loading: %s\n", fileName.c_str());

    auto x = fileName.find(foreignSeparator());
    assert(x == std::string::npos);

    float* pSampleData = drwav_open_file_and_read_pcm_frames_f32(fileName.c_str(), &numChannels, &sampleRate, &totalFrameCount, nullptr);
    if (pSampleData == NULL) {
        // Error opening and reading WAV file.
        SQWARN("error opening wave\n");
        return;
    }
    SQINFO("after load, frames = %lld rate= %d ch=%d\n", totalFrameCount, sampleRate, numChannels);
    data = pSampleData;
    valid = true;
}

char WaveLoader::nativeSeparator() {
#ifdef ARCH_WIN
    return '\\';
#else
    return '/';
#endif
}

char WaveLoader::foreignSeparator() {
#ifdef ARCH_WIN
    return '/';
#else
    return '\\';
#endif
}

void WaveLoader::makeAllSeparatorsNative(std::string& s) {
    std::replace(s.begin(), s.end(), foreignSeparator(), nativeSeparator());
}

WaveLoader::WaveInfo::~WaveInfo() {
    if (data) {
        drwav_free(data, nullptr);
        data = nullptr;
    }
}
