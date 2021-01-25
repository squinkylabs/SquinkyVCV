
#include "WaveLoader.h"

#include <assert.h>

#include <algorithm>

#include "SqLog.h"

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
    auto x = fileName.find(foreignSeparator());
    assert(x == std::string::npos);
    filesToLoad.push_back(fileName);
}

bool WaveLoader::load() {
    assert(!didLoad);
    didLoad = true;
    for (std::string& file : filesToLoad) {
        WaveInfoPtr waveInfo = std::make_shared<WaveInfo>(file);
        const bool b = waveInfo->load();
        if (!b) {
            // bail on first error
            return false;
        }

        finalInfo.push_back(waveInfo);
    }
    return true;
}

bool WaveLoader::WaveInfo::load() {
    SQINFO("loader started loading waves");
    auto x = fileName.find(foreignSeparator());
    assert(x == std::string::npos);

    float* pSampleData = drwav_open_file_and_read_pcm_frames_f32(fileName.c_str(), &numChannels, &sampleRate, &totalFrameCount, nullptr);
    if (pSampleData == NULL) {
        // Error opening and reading WAV file.
        SQWARN("error opening wave %s", fileName.c_str());
        return false;
    }
    //SQINFO("after load, frames = %lld rate= %d ch=%d\n", totalFrameCount, sampleRate, numChannels);
    data = pSampleData;
    if (numChannels > 1) {
       // pSampleData = convertToMono(pSampleData, totalFrameCount, numChannels);
      //  numChannels = 1;
        convertToMono();
    }
    //data = pSampleData;
    valid = true;
    SQINFO("loader loaded all wave files");
    return true;
}

void WaveLoader::WaveInfo::convertToMono() {
   // SQINFO("converting to mono from %d %d", numChannels, sampleRate);
    const int origChannels = numChannels;
    uint64_t newBufferSize = 1 + totalFrameCount / origChannels;
    void* x = DRWAV_MALLOC(newBufferSize * sizeof(float));
    float* dest = reinterpret_cast<float*>(x);
    for (uint64_t i = 0; i < totalFrameCount / origChannels; ++i) {
        float temp = 0;
        for (int ch = 0; ch < origChannels; ++ch) {
            temp += data[i + ch];
        }
        temp /= origChannels;
        assert(temp <= 1);
        assert(temp >= -1);
        dest[i] = temp;
    }

    totalFrameCount /= origChannels;
    numChannels = 1;

    DRWAV_FREE(data);
    data = dest;
}

#if 0
float* WaveLoader::WaveInfo::convertToMono(float* data, uint64_t frames, int channels) {
    uint64_t newBufferSize = 1 + frames / channels;
    void* x = DRWAV_MALLOC(newBufferSize * sizeof(float));
    float* dest = reinterpret_cast<float*>(x);
    for (uint64_t i = 0; i < frames / channels; ++i) {
        float temp = 0;
        for (int ch = 0; ch < channels; ++ch) {
            temp += data[i + ch];
        }
        temp /= channels;
        assert(temp <= 1);
        assert(temp >= -1);
        dest[i] = temp;
    }

    DRWAV_FREE(data);
    return dest;
}
#endif

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
