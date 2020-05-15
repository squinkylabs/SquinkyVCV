
/**
 * include the "foreign" SIMD library
 */


// Need to make this compile in MS tools for unit tests
#if defined(_MSC_VER)
//#define __attribute__(x)

#pragma warning (push)
#pragma warning ( disable: 4244 4305 )
#endif

#include <algorithm>
#include <cstdint>
#include <simd/vector.hpp>
#include <simd/functions.hpp>


#if defined(_MSC_VER)
#pragma warning (pop)
#endif