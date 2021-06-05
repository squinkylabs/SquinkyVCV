
#include "FlacReader.h"
#include "SqLog.h"
#include "WaveLoader.h"
#include "share/windows_unicode_filenames.h"

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

class LoaderBase : public WaveInfoInterface {
public:
    LoaderBase(const FilePath& _fp) : fp(_fp) {}
    unsigned int getSampleRate() override { return sampleRate; }
    uint64_t getTotalFrameCount() override { return totalFrameCount; }
    const float* getData() override { return data; }
    bool isValid() const override { return valid; }
    std::string getFileName() override { return fp.toString(); }

protected:
    unsigned int sampleRate = 0;
    uint64_t totalFrameCount = 0;

    // Who owns this data? I think I should own it, and delete it myself. I do,
    // but should I transfer ownership to outer object?
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
    float* loadData(unsigned& numChannels);          // no format conversion or checking
};

#ifdef ARCH_WIN

//extern "C" {
//    extern wchar_t* wchar_from_utf8(const char* str);
//}
float* WaveFileLoader::loadData(unsigned& numChannels) {
    wchar_t* widePath = wchar_from_utf8(fp.toString().c_str());
    float* ret = drwav_open_file_and_read_pcm_frames_f32_w(widePath, &numChannels, &sampleRate, &totalFrameCount, nullptr);
    free(widePath);
    return ret;
}
#else
float* WaveFileLoader::loadData(unsigned& numChannels) {
    return drwav_open_file_and_read_pcm_frames_f32(fp.toString().c_str(), &numChannels, &sampleRate, &totalFrameCount, nullptr);
}
#endif

bool WaveFileLoader::load(std::string& errorMessage) {
    unsigned int numChannels = 0;

    float* pSampleData = loadData(numChannels);
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
            case WaveLoader::Tests::RampOneSec:
                setupRamp(1);
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
    void setupRamp(int seconds) {
        data = reinterpret_cast<float*>(DRWAV_MALLOC(44100 * seconds * sizeof(float)));
        sampleRate = 44100;
        totalFrameCount = 44100 * seconds;
        for (uint64_t i = 0; i < totalFrameCount; ++i) {
            data[i] = float(i);
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

        errorMsg = "unable to load ." + fp.getExtensionLC() + " file type.";
        return false;
    }
};

WaveLoader::WaveInfoPtr WaveLoader::loaderFactory(const FilePath& file) {
    WaveLoader::WaveInfoPtr loader;

    const std::string extension = file.getExtensionLC();
    assert(extension.find('\n') == extension.npos);
    assert(extension.find('\r') == extension.npos);
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
    WaveInfoPtr wav = std::make_shared<TestFileLoader>(FilePath(), test);
    switch (test) {
        case Tests::None:
            break;
        case Tests::RampOneSec:
        case Tests::DCTenSec:
        case Tests::DCOneSec: {
            finalInfo.push_back(wav);
            std::string err;
            const bool b = wav->load(err);
            assert(b);
            didLoad = true;
        } break;
        default:
            assert(false);
    }
}