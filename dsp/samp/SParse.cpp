
#include "SParse.h"

#include <assert.h>

// global for mem leak detection
int parseCount = 0;

std::string SParse::go(const std::string& s, SInstrumentPtr) {
    assert(false);
    return "fail";
}

std::string SParse::goFile(const FilePath& filePath, SInstrumentPtr)  {
    assert(false);
    return "fail";
}