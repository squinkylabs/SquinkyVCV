#pragma once

#include <memory>
#include <string>
#include <vector>

#include "SamplerSchema.h"
#include "SqLog.h"

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

// A heading represents a <global> simiar
// It doesn't have much, other than a list of value
class SHeading {
public:
    SKeyValueList values;
    SamplerSchema::KeysAndValuesPtr compiledValues;
    static void dumpKeysAndValues(const SKeyValueList& v);
    int lineNumber = 0;
};

class SRegion {
public:
    // TODO: process cb contents
    SRegion(int line, const SHeading& controlBlockContents) : lineNumber(line) {
        ++parseCount;
        values.insert(values.end(), controlBlockContents.values.begin(), controlBlockContents.values.end());
    }
    ~SRegion() { --parseCount; }
    SRegion(const SRegion&) = delete;
    SKeyValueList values;
    SamplerSchema::KeysAndValuesPtr compiledValues;
    const int lineNumber = 0;
    void _dump();
};
using SRegionPtr = std::shared_ptr<SRegion>;
using SRegionList = std::vector<SRegionPtr>;

// Groups have a list of values
// and they usually have children.
// they are a type of heading, but they are special
class SGroup : public SHeading {
public:
    SGroup(int line) : lineNumber(line) {}
    SGroup() = delete;

    SRegionList regions;
    const int lineNumber;
    void _dump();
};

using SGroupPtr = std::shared_ptr<SGroup>;
using SGroupList = std::vector<SGroupPtr>;

class SLex;
class SLexItem;
class SInstrument;
using SLexPtr = std::shared_ptr<SLex>;
using SLexItemPtr = std::shared_ptr<SLexItem>;
using SInstrumentPtr = std::shared_ptr<SInstrument>;

class SParse {
public:
    static std::string go(const std::string& s, SInstrumentPtr);
    static std::string goFile(const FilePath& filePath, SInstrumentPtr);
   

private:
    class Result {
    public:
        std::string errorMessage;
        enum Res {
            ok,        // matched
            no_match,  // finished matching
            error
        };
        Res res = Res::ok;
    };

    static std::shared_ptr<std::ifstream> open(const FilePath& fp);

    static std::string goCommon(const std::string& sContent, SInstrumentPtr outParsedInstrument, const FilePath* fullPathToSFZ);

    /* What is a "heading"?
     * it's something that can modify following regions: group, control, etc...
     * we will treat these all pretty much the same
     */

    static std::string matchRegions(SRegionList&, SLexPtr, const SHeading& controlBlock);
    static Result matchRegion(SRegionList&, SLexPtr, const SHeading& controlBlock);

    //  static std::string matchHeadings(SInstrumentPtr, SLexPtr lex);
    // a"heading group" is a series of headings followed by all regions that belong to it
    static std::string matchHeadingGroups(SInstrumentPtr, SLexPtr lex);

    static Result matchHeadingGroup(SInstrumentPtr, SLexPtr);
    //  static std::string matchHeadingsOrRegions(SInstrumentPtr, SLexPtr lex);

    // return.second is true it heading it a group
    static std::pair<Result, bool> matchSingleHeading(SInstrumentPtr, SLexPtr);

    static std::string matchKeyValuePairs(SKeyValueList&, SLexPtr);
    static Result matchKeyValuePair(SKeyValueList&, SLexPtr);

    // return empty if it's not a tag
    static std::string getTagName(SLexItemPtr);
};