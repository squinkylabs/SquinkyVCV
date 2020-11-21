
/**
 * include the "foreign" SIMD libraries (from VCV)
 */

// Need to make this compile in MS tools for unit tests
#if defined(_MSC_VER)
//#define __attribute__(x)

#pragma warning (push)
#pragma warning ( disable: 4244 4305 )
#define NOMINMAX
#endif

#include <algorithm>
#include <cstdint>
#include <simd/vector.hpp>
#include <simd/functions.hpp>
#include "SqMath.h"
#include "dsp/common.hpp"
#include "dsp/approx.hpp"

using float_4 = rack::simd::float_4;
using int32_4 = rack::simd::int32_4;


#if defined(_MSC_VER)
#pragma warning (pop)
#endif