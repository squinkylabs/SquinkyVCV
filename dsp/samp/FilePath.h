#pragma once

#include <string>

class FilePath {
public:
    FilePath(const char*);
    FilePath(const std::string&);
    std::string toString() const;

    static char nativeSeparator();
    static char foreignSeparator();
    //static void makeAllSeparatorsNative(std::string& s);
private:
    std::string data;

    void fixSeparators();
};
