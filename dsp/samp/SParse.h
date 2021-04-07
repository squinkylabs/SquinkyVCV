
#pragma once

#include <memory>
#include <string>
#include <vector>

class SLex;
class SLexItem;
class SInstrument;
using SLexPtr = std::shared_ptr<SLex>;
using SLexItemPtr = std::shared_ptr<SLexItem>;
using SInstrumentPtr = std::shared_ptr<SInstrument>;

extern int parseCount;
class FilePath;

class SKeyValuePair {
public:
    SKeyValuePair(const std::string& k, const std::string& v) : key(k), value(v) { ++parseCount; }
    SKeyValuePair() { ++parseCount; }
    ~SKeyValuePair() { --parseCount; }
    std::string key;
    std::string value;
};
using SKeyValuePairPtr = std::shared_ptr<SKeyValuePair>;
using SKeyValueList = std::vector<SKeyValuePairPtr>;

class SParse {
public:
    static std::string go(const std::string& s, SInstrumentPtr);
    static std::string goFile(const FilePath& filePath, SInstrumentPtr);
};