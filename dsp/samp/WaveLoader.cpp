
#include "WaveLoader.h"
#include "SqLog.h"

#include <assert.h>
#include <algorithm>

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
    curLoadIndex = 0;
}

float WaveLoader::getProgressPercent() const {
    float done = float(curLoadIndex);
    float total = float(filesToLoad.size());
    return 100 * done / total;
}

WaveLoader::LoaderState WaveLoader::loadNextFile() {
    if (curLoadIndex >= int(filesToLoad.size())) {
        return LoaderState::Done;
    }

    FilePath& file = filesToLoad[curLoadIndex];
    WaveInfoPtr fileLoader = loaderFactory(file);
    std::string err;
    const bool b = fileLoader->load(err);
    if (!b) {
        // bail on first error
        assert(!err.empty());
        lastError = err;
        SQINFO("wave loader leaving with error %s", lastError.c_str());
        return LoaderState::Error;
    }

    finalInfo.push_back(fileLoader);
    curLoadIndex++;
    auto ret = curLoadIndex >= int(filesToLoad.size()) ? LoaderState::Done : LoaderState::Progress;
    if (ret == LoaderState::Done) {
        didLoad = true;
    }
    return ret;
}
