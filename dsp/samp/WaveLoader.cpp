

#include "WaveLoader.h"
#include <assert.h>

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#if 0
/*
typedef struct
{
    void* pUserData;
    void* (* onMalloc)(size_t sz, void* pUserData);
    void* (* onRealloc)(void* p, size_t sz, void* pUserData);
    void  (* onFree)(void* p, void* pUserData);
} drwav_allocation_callbacks;
*/

drwav_allocation_callbacks mycb;
bool mycb_init = false;

void initmycb()
{
    if (mycb_init) {
        return;
    }
    mycb.onMalloc = [](size_t sz, void* pUserData) {
       // 
        auto ret =  malloc(sz);
        printf("malloc(%zd) ret %p\n", sz, ret); fflush(stdout);
        return ret;
    };
    mycb.onRealloc = [](void * p, size_t sz, void* pUserData) {
        auto ret =  realloc(p, sz);
        printf("realloc(%p, %zd) ret %p\n", p, sz, ret); fflush(stdout);
        return ret;
    };
    mycb.onFree = [](void * p, void* pUserData) {
       printf("free(%p)\n", p);  fflush(stdout);
        return free(p);
    };
    mycb_init = true;
}
#endif

#if 0
void WaveLoader::load(const std::string& fileName) {
    clear();
    WaveInfoPtr wi = std::make_shared<WaveInfo>(fileName);
   
    wi->load();
    info.push_back(wi);
}
#endif

void WaveLoader::clear()
{
    finalInfo.clear();
}

WaveLoader::WaveInfoPtr WaveLoader::getInfo(int index) const {
    assert(didLoad);
    if (index < 1 || index > int(finalInfo.size())) {
        return nullptr;
    }
    return finalInfo[index-1];

}

WaveLoader::WaveInfo::WaveInfo(const std::string& path) : fileName(path) {
    //initmycb();
}

void WaveLoader::addNextSample(const std::string& fileName)
{
    assert(!didLoad);
    printf("adding %s\n", fileName.c_str());
    filesToLoad.push_back(fileName);
}

void WaveLoader::load()
{
    printf("waveLoader::load %lld\n", filesToLoad.size());
    assert(!didLoad);
    didLoad = true;
    for (std::string& file : filesToLoad) {
        WaveInfoPtr waveInfo = std::make_shared<WaveInfo>(file);
        waveInfo->load();

        finalInfo.push_back(waveInfo);
        printf("adding one\n");
    }
}

void WaveLoader::WaveInfo::load() {
    printf("loading: %s\n", fileName.c_str());
    float* pSampleData = drwav_open_file_and_read_pcm_frames_f32(fileName.c_str(), &numChannels, &sampleRate, &totalFrameCount, nullptr);
    if (pSampleData == NULL) {
        // Error opening and reading WAV file.
        printf("error opening wave\n");
        return;
    }
    printf("after load, frames = %lld rate= %d ch=%d\n", totalFrameCount, sampleRate, numChannels); fflush(stdout);
    data = pSampleData;
    valid = true;
}

WaveLoader::WaveInfo::~WaveInfo() {
    if (data) {
          drwav_free(data, nullptr);
          data = nullptr;
    }
}