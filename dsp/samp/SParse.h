
#pragma once

#include "SamplerSchema.h"

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

//---------------------------------------------
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

//-----------------------------------------------------
// A heading represents any heading, including regions and groups
class SHeading {
public:
    enum class Type {
        Region,
        Group,
        Unknown,
        NUM_TYPES
    };
    /**
     * Parsing populates values with the opcodes found while parsing
     */
    SKeyValueList values;

    /**
     * A step in compiling is turning values into compiledValues.
     * This is fairly mechanical, and driven from SamperSchema
     */
    SamplerSchema::KeysAndValuesPtr compiledValues;
    static void dumpKeysAndValues(const SKeyValueList& v);
    int lineNumber = 0;
    Type type = {Type::Unknown};
};

//-------------------------------------------
class SParse {
public:
    static std::string go(const std::string& s, SInstrumentPtr);
    static std::string goFile(const FilePath& filePath, SInstrumentPtr);

private:
    static FILE* openFile(const FilePath& fp);
    static std::string readFileIntoString(FILE* fp);
    static std::string goCommon(const std::string& sContent, SInstrumentPtr outParsedInstrument, const FilePath* fullPathToSFZ);
};