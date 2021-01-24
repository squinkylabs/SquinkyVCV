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

    void concat(const FilePath& other);
private:
    std::string data;

    void fixSeparators();
    bool startsWithSeparator() const;
    bool endsWithSeparator() const;
};
