
#include "WaveLoader.h"

#include <assert.h>

#include <algorithm>

#include "SqLog.h"

// Instantiate the dr_wav functions in this file
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

void WaveLoader::addNextSample(const FilePath& fileName) {
    assert(!didLoad);
    filesToLoad.push_back(fileName);
}

bool WaveLoader::load() {
    assert(!didLoad);
    didLoad = true;
    SQINFO("loader started loading waves");
    for (FilePath& file : filesToLoad) {
        // SQINFO("wave loader loading %s", file.c_str());
        WaveInfoPtr waveInfo = std::make_shared<WaveInfo>(file);
        std::string err;
        const bool b = waveInfo->load(err);
        if (!b) {
            // bail on first error
            assert(!err.empty());
            lastError = err;
            SQINFO("wave loader leaving with error %s", lastError.c_str());
            return false;
        }

        finalInfo.push_back(waveInfo);
    }
    SQINFO("loader loaded all wave files");
#ifndef NDEBUG
    validate();
#endif
    return true;
}

void WaveLoader::validate()
{
    for (auto info : finalInfo) {
        info->validate();
    }
}

void WaveLoader::_setTestMode(Tests test) {
    _testMode = test;
    switch (_testMode) {
        case Tests::None:
            break;
        case Tests::DCTenSec:
        case Tests::DCOneSec: {
            auto info = std::make_shared<WaveInfo>(_testMode);
            finalInfo.push_back(info);
            didLoad = true;
        } break;
        default:
            assert(false);
    }
}

//***********************************************************************************************************************

WaveLoader::WaveInfo::WaveInfo(const FilePath& path) : fileName(path) {
}


WaveLoader::WaveInfo::WaveInfo(Tests test) : fileName(FilePath("test only")) {
    //  assert(test == Tests::DCOneSec);        // only one imp right now
    assert(!data);
    int framesMult = 1;
    switch (test) {
        case Tests::DCOneSec:
            framesMult = 1;
            break;
        case Tests::DCTenSec:
            framesMult = 10;
            break;
        default:
            assert(false);
    }
    const int frames = 44100 * framesMult;
    data = reinterpret_cast<float*>(malloc(frames * sizeof(float)));
    assert(data);

    for (int i = 0; i < frames; ++i) {
        data[i] = 1.f;
    }
    valid = true;
    numChannels = 1;
    sampleRate = 44100;
    totalFrameCount = frames;
    // fileName = FilePath("test only name");
}

bool WaveLoader::WaveInfo::load(std::string& errorMessage) {
   // SQINFO("loading %s", fileName.toString().c_str());
    float* pSampleData = drwav_open_file_and_read_pcm_frames_f32(fileName.toString().c_str(), &numChannels, &sampleRate, &totalFrameCount, nullptr);
    if (pSampleData == NULL) {
        // Error opening and reading WAV file.
        errorMessage += "can't open ";
        errorMessage += fileName.getFilenamePart();
        SQWARN("error opening wave %s", fileName.toString().c_str());
        return false;
    }
    //SQINFO("after load, frames = %lld rate= %d ch=%d\n", totalFrameCount, sampleRate, numChannels);
    data = pSampleData;
    if (numChannels > 1) {
        convertToMono();
    }
    valid = true;
    return true;
}


void WaveLoader::WaveInfo::convertToMono() {
  //  SQINFO("convert to mono. file=%s channels=%d totalFrameCount=%d", fileName.getFilenamePart().c_str(), numChannels, totalFrameCount);
    const int origChannels = numChannels;
    uint64_t newBufferSize = 1 + totalFrameCount;
    void* x = DRWAV_MALLOC(newBufferSize * sizeof(float));
    float* dest = reinterpret_cast<float*>(x);

    for (uint64_t outputIndex = 0; outputIndex < totalFrameCount; ++outputIndex) {
        float monoSampleValue = 0;
        for (int channelIndex=0; channelIndex < origChannels; ++channelIndex) {
            uint64_t inputIndex = outputIndex * origChannels + channelIndex;
            monoSampleValue += data[inputIndex];
        }
        monoSampleValue /= origChannels;
        assert(monoSampleValue <= 1);
        assert(monoSampleValue >= -1);
        dest[outputIndex] = monoSampleValue;
    }
    numChannels = 1;
  //  SQINFO("leaving, not total frames = %d", totalFrameCount);
    DRWAV_FREE(data);
    data = dest;
}

WaveLoader::WaveInfo::~WaveInfo() {
    if (data) {
        drwav_free(data, nullptr);
        data = nullptr;
    }
}
