#pragma once



/**
 * This awful hack is so that both the real plugin and
 * the unit tests can pass this "Output" struct around
 * 
 * TODO: move to a common include file??
 */


#ifdef __PLUGIN
namespace rack {
namespace engine {
struct Input;
struct Param;
struct Output;
}
}  // namespace rack
#else
#include "TestComposite.h"
#endif


#ifdef __PLUGIN
    using SqInput = rack::engine::Input;
    using SqOutput = rack::engine::Output;
    using SqParam = rack::engine::Param;
#else
    using SqOutput = ::Output;
    using SqInput = ::Input;
    using SqParam = ::Param;
#endif