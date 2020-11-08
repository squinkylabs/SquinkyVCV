#pragma once

#include <assert.h>
#include <string>
class SqStream
{
public:
    SqStream();
    void add(const std::string& s);
    void add(float f);
    void add(int i);
    void add(const char* s);
    std::string str();

    void setPrecission(int digits);
private:
    static const int bufferSize = 256;
    char buffer[bufferSize];
    int length = 0;

    const int precission = 2;
};

inline SqStream::SqStream()
{
    buffer[0] = 0;
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
    length = int(strlen(buffer));
}

inline void SqStream::add(float f)
{
    char* nextLoc = buffer + length;
    int sizeRemaining = bufferSize - length;
    assert(sizeRemaining > 0);
    assert(precission == 2);
    snprintf(nextLoc, sizeRemaining, "%.2f", f);
    length = int(strlen(buffer));
}
inline std::string SqStream::str()
{
    return buffer;
}
