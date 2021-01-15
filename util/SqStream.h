#pragma once

#include "SqLog.h"

#include <assert.h>
#include <string>
#include <cstring>

/**
 * SqStream is a replacement for std::stringstream.
 * The std one crashes some build of rack.
 * SqStream is not drop in compatibly. Instead of << you must
 * use add().
 */
class SqStream
{
public:
    SqStream();
    void add(const std::string& s);
    void add(float f);
    void add(double d);
    void add(int i);
    void add(const char* s);
    std::string str();

    void precision(int digits);
private:
    static const int bufferSize = 256;
    char buffer[bufferSize];
    int length = 0;

    int _precision = 2;
};

inline SqStream::SqStream()
{
    buffer[0] = 0;
}

inline void SqStream::precision(int p)
{
    _precision = p;
}
inline void SqStream::add(const std::string& s)
{
    add(s.c_str());
}

inline void SqStream::add(const char *s)
{
    char* nextLoc = buffer + length;
    int sizeRemaining = bufferSize - length;
    assert(sizeRemaining > 0);
    snprintf(nextLoc, sizeRemaining, "%s", s);
    length = int(std::strlen(buffer));
}

inline void SqStream::add(int i)
{
    char* nextLoc = buffer + length;
    int sizeRemaining = bufferSize - length;
    assert(sizeRemaining > 0);
    snprintf(nextLoc, sizeRemaining, "%d", i);
    length = int(strlen(buffer));
}

inline void SqStream::add(double d)
{
    add( float(d));
}

inline void SqStream::add(float f)
{
    char* nextLoc = buffer + length;
    int sizeRemaining = bufferSize - length;
    assert(sizeRemaining > 0);

    const char* format = "%.2f";
    switch (_precision) {
    case 0:
        format = "%.0f";
        break; 
    case 1:
        format = "%.1f";
        break;
    case 2:
        format = "%.2f";
        break;
    default:
        SQWARN("unimplemented precission %d\n", _precision);

    }

    snprintf(nextLoc, sizeRemaining, format, f);
    length = int(strlen(buffer));
    SQWARN("float was %f, printed to %s prec=%d\n", f, nextLoc, _precision);
}
inline std::string SqStream::str()
{
    return buffer;
}
