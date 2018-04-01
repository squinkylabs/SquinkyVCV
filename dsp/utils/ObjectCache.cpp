
#include <assert.h>

#include "LookupTableFactory.h"
#include "ObjectCache.h"


std::shared_ptr<LookupTableParams<float>> ObjectCache::getBipolarAudioTaper()
{
    std::shared_ptr< LookupTableParams<float>> ret = bipolarAudioTaper.lock();
    if (!ret) {
        ret = std::make_shared<LookupTableParams<float>>();
        LookupTableFactory<float>::makeBipolarAudioTaper(*ret);
        bipolarAudioTaper = ret;
    }
    return ret;
}


std::weak_ptr<LookupTableParams<float>> ObjectCache::bipolarAudioTaper;