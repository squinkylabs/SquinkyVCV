#pragma once


#include "LookupTable.h"

/**
 * This class creates objects and caches them.
 * Objects in the cache only stay alive as long as there is a reference to the object,
 * If all refs go away, the object will be deleted.
 *
 * All accessors return shared pointers to make the lifetime management easy.
 * Clients are free to use the shared_ptr directly, or may use the raw pointer,
 * as long as the client holds onto the reference.
 */

template <typename T>
class ObjectCache
{
public:
    static std::shared_ptr<LookupTableParams<T>> getBipolarAudioTaper();
    static std::shared_ptr<LookupTableParams<T>> getSinLookup();
    static std::shared_ptr<LookupTableParams<T>> getExp2();
    static std::shared_ptr<LookupTableParams<T>> getDb2Gain();

private:
    /**
     * Cache uses weak pointers. This allows the cached objects to be 
     * freed when the last client reference goes away.
     */
    static std::weak_ptr<LookupTableParams<T>> bipolarAudioTaper;
    static std::weak_ptr<LookupTableParams<T>> sinLookupTable;
    static std::weak_ptr<LookupTableParams<T>> exp2;
    static std::weak_ptr<LookupTableParams<T>> db2Gain;
};
