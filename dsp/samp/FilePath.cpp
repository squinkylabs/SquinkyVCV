
#include "FilePath.h"

#include <algorithm>
#include <assert.h>

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

void FilePath::concat(const FilePath& other) {
    const bool firstEndsWithSeparator = this->endsWithSeparator();
    const bool secondStartsWithSeparator = other.startsWithSeparator();
    if (firstEndsWithSeparator && secondStartsWithSeparator) {
        data.pop_back();
        data += other.data;
    } else if (firstEndsWithSeparator || secondStartsWithSeparator) {
        data += other.data;
    } else {
        data += nativeSeparator();
        data += other.data;
    }
}

bool FilePath::startsWithSeparator() const {
    if (data.empty()) {
        return false;
    }
    return data.at(0) == nativeSeparator();
}

bool FilePath::endsWithSeparator() const
{
     if (data.empty()) {
        return false;
    }
    return data.back() == nativeSeparator();
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
