#pragma once

/**
 * Microsoft compiler does not have x86intrin.h, we we need to make one
 * This file is only in the search path for MS builds.
 */

#include <xmmintrin.h>
#include <mmintrin.h>
#include <immintrin.h>