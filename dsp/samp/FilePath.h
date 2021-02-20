#pragma once

#include <string>

class FilePath {
public:
    FilePath(const char*);
    FilePath(const std::string&);
    FilePath() = default;
    std::string toString() const;

    static char nativeSeparator();
    static char foreignSeparator();
    //static void makeAllSeparatorsNative(std::string& s);

    /**
     * a.concat(b) -> a + b (with some fixups)
     */
    void concat(const FilePath& other);
    bool empty() const;

    // if this == "abc/def/j.txt"
    // will return abc/def
    FilePath getPathPart() const;
private:
    std::string data;

    void fixSeparators();
    bool startsWithSeparator() const;
    bool endsWithSeparator() const;
    bool startsWithDot() const;
};
