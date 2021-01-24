
#include "FilePath.h"

#include <algorithm>

FilePath::FilePath(const char* s) : data(s) {
    fixSeparators();
}

FilePath::FilePath(const std::string& s) : data(s) {
    fixSeparators();
}

std::string FilePath::toString() const {
    return data;
}

void FilePath::fixSeparators() {
    std::replace(data.begin(), data.end(), foreignSeparator(), nativeSeparator()); 
}



char FilePath::nativeSeparator() {
#ifdef ARCH_WIN
    return '\\';
#else
    return '/';
#endif
}

char FilePath::foreignSeparator() {
#ifdef ARCH_WIN
    return '/';
#else
    return '\\';
#endif
}