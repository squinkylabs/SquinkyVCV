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

class ObjectCache
{
public:
    static std::shared_ptr<LookupTableParams<float>> getBipolarAudioTaper();

private:
    static std::weak_ptr<LookupTableParams<float>> bipolarAudioTaper;
};
