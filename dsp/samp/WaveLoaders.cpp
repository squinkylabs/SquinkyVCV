
#include "FlacReader.h"
#include "SqLog.h"
#include "WaveLoader.h"

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

/*
class WaveInfoInterface {
public:
    virtual ~WaveInfoInterface() = default;
    virtual unsigned int getSampleRate() = 0;
    virtual uint64_t getTotalFrameCount() = 0;
    virtual bool isValid() = 0;
    virtual const float* getData() = 0;
    virtual bool load(std::string& errorMsg) = 0;
};
*/

class LoaderBase : public WaveInfoInterface {
public:
    LoaderBase(const FilePath& _fp) : fp(_fp) {}
    unsigned int getSampleRate() override { return sampleRate; }
    uint64_t getTotalFrameCount() override { return totalFrameCount; }
    const float* getData() override { return data; }
    bool isValid() const override { return valid; }
    std::string getFileName() { return fp.toString(); }

protected:
    unsigned int sampleRate = 0;
    uint64_t totalFrameCount = 0;

    // who owns this data. I think I should own it, and delete it maysel. I do,
    // but should I transer ownership to outer object?
    // Or maybe I should keep it and outer caller gets it to play?
    float* data = nullptr;
    const FilePath fp;
    bool valid = false;
};

//---------------------------------------------------------------
class WaveFileLoader : public LoaderBase {
public:
    WaveFileLoader(const FilePath& fp) : LoaderBase(fp) {}
    bool load(std::string& errorMsg) override;

private:
    void convertToMono();
};

bool WaveFileLoader::load(std::string& errorMessage) {
    unsigned int numChannels = 0;
    //  unsigned int sampleRate = 0;
    //   uint64_t totalFrameCount = 0;

    float* pSampleData = drwav_open_file_and_read_pcm_frames_f32(fp.toString().c_str(), &numChannels, &sampleRate, &totalFrameCount, nullptr);
    if (pSampleData == NULL) {
        // Error opening and reading WAV file.
        errorMessage += "can't open ";
        errorMessage += fp.getFilenamePart();
        SQWARN("error opening wave %s", fp.toString().c_str());
        return false;
    }
    if (numChannels != 1 && numChannels != 2) {
        errorMessage += "unsupported channel number in ";
        errorMessage += fp.getFilenamePart();
        return false;
    }
    // TODO: insist that they be 2
    //SQINFO("after load, frames = %lld rate= %d ch=%d\n", totalFrameCount, sampleRate, numChannels);
    data = pSampleData;
    if (numChannels == 2) {
        convertToMono();
    }
    valid = true;
    return true;
}

void WaveFileLoader::convertToMono() {
    //  SQINFO("convert to mono. file=%s channels=%d totalFrameCount=%d", fileName.getFilenamePart().c_str(), numChannels, totalFrameCount);
    //  const int origChannels = numChannels;
    uint64_t newBufferSize = 1 + totalFrameCount;
    void* x = DRWAV_MALLOC(newBufferSize * sizeof(float));
    float* dest = reinterpret_cast<float*>(x);

    for (uint64_t outputIndex = 0; outputIndex < totalFrameCount; ++outputIndex) {
        float monoSampleValue = 0;
        for (int channelIndex = 0; channelIndex < 2; ++channelIndex) {
            uint64_t inputIndex = outputIndex * 2 + channelIndex;
            monoSampleValue += data[inputIndex];
        }
        monoSampleValue *= .5f;
        assert(monoSampleValue <= 1);
        assert(monoSampleValue >= -1);
        dest[outputIndex] = monoSampleValue;
    }

    //  SQINFO("leaving, not total frames = %d", totalFrameCount);
    DRWAV_FREE(data);
    data = dest;
}

//----------------------------------------------------------------
class FlacFileLoader : public LoaderBase {
public:
    FlacFileLoader(const FilePath& fp) : LoaderBase(fp) {}

    bool load(std::string& errorMsg) override {
        reader.read(fp);
        if (reader.ok()) {
            valid = true;
            data = reader.takeSampleBuffer();
            sampleRate = reader.getSampleRate();
            totalFrameCount = reader.getTotalFrameCount();

            return true;
        }
        errorMsg = "can't open " +  fp.getFilenamePart();
        return false;
    }

private:
    FlacReader reader;
};

//-------------------------------------------
class TestFileLoader : public LoaderBase {
public:
    TestFileLoader(const FilePath& fp, WaveLoader::Tests test) : LoaderBase(fp), _test(test) { valid = true; }

    bool load(std::string& errorMsg) override {
        switch (_test) {
            case WaveLoader::Tests::DCOneSec:
                setupDC(1);
                break;
            case WaveLoader::Tests::DCTenSec:
                setupDC(10);
                break;
            default:
                assert(false);
        }
        return true;
    }

private:
    void setupDC(int seconds) {
        data = reinterpret_cast<float*>(DRWAV_MALLOC(44100 * seconds * sizeof(float)));
        sampleRate = 44100;
        totalFrameCount = 44100 * seconds;
        for (uint64_t i = 0; i < totalFrameCount; ++i) {
            data[i] = 1;
        }
    }
    const WaveLoader::Tests _test = WaveLoader::Tests::None;
};

//-------------------------------------------------
class NullFileLoader : public LoaderBase {
public:
    NullFileLoader(const FilePath& fp) : LoaderBase(fp) {}
    bool isValid() {
        assert(false);
        return false;
    }
    bool load(std::string& errorMsg) override {
        //assert(false);
        errorMsg = "nimp";
        return false;
    }
};

WaveLoader::WaveInfoPtr WaveLoader::loaderFactory(const FilePath& file) {
    WaveLoader::WaveInfoPtr loader;

    const std::string extension = file.getExtensionLC();
    if (extension == "wav") {
        loader = std::make_shared<WaveFileLoader>(file);
    } else if (extension == "flac") {
        loader = std::make_shared<FlacFileLoader>(file);
    } else {
        loader = std::make_shared<NullFileLoader>(file);
    }

    return loader;
}

void WaveLoader::_setTestMode(Tests test) {
    //  _testMode = test;
    WaveInfoPtr wav = std::make_shared<TestFileLoader>(FilePath(), test);
    switch (test) {
        case Tests::None:
            break;
        case Tests::DCTenSec:
        case Tests::DCOneSec: {
            // auto info = std::make_shared<TestFileLoader>(FilePath(), test);
            finalInfo.push_back(wav);
            std::string err;
            const bool b = wav->load(err);
            assert(b);
            //   addNextSample(info);
            didLoad = true;
        } break;
        default:
            assert(false);
    }
#if 0
    //  curLoadIndex = 0;       // let's force a load
    for (bool done = false; !done;) {
        WaveLoader::LoaderState loadedState = this->loadNextFile();
        switch (loadedState) {
            case WaveLoader::LoaderState::Progress:
                // smsg->sharedState->uiw_setLoadProgress(waves->getProgressPercent());
                break;
            case WaveLoader::LoaderState::Done:
            case WaveLoader::LoaderState::Error:
                done = true;
                break;
            default:
                assert(false);
        }
    }
#endif
}