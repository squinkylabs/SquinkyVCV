

#include "WaveLoader.h"
#include <assert.h>

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

void WaveLoader::load(const std::string& fileName) {
    clear();
    WaveInfo wi(fileName);
   
    wi.load();
    info.push_back(wi);
    printf("after push, there are %d entries\n", int(info.size())); fflush(stdout);


}

void WaveLoader::clear()
{
    info.clear();
}

const WaveLoader::WaveInfo& WaveLoader::getInfo(int index) const {
    assert(index < info.size());
    return info[index];

}

WaveLoader::WaveInfo::WaveInfo(const std::string& path) : fileName(path) {

}

void WaveLoader::WaveInfo::load() {
    printf("loading: %s\n", fileName.c_str());
    float* pSampleData = drwav_open_file_and_read_pcm_frames_f32(fileName.c_str(), &numChannels, &sampleRate, &totalFrameCount, NULL);
    if (pSampleData == NULL) {
        // Error opening and reading WAV file.
        printf("error opening wave\n");
        return;
    }
    data = pSampleData;
    valid = true;
}

WaveLoader::WaveInfo::~WaveInfo() {
    if (data) {
        fprintf(stderr, "leaking memory, please fix\n");
         // drwav_free(data, nullptr);
          data = nullptr;
    }
}