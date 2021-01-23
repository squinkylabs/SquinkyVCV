#pragma once

#include <memory>
#include <string>
#include <vector>

#include "SamplerSchema.h"

extern int parseCount;

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

class SRegion {
public:
    SRegion(int line) : lineNumber(line) { ++parseCount; }
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
class SGroup {
public:
    SGroup(int line) : lineNumber(line) {}
    SGroup() = delete;

    SKeyValueList values;
    SamplerSchema::KeysAndValuesPtr compiledValues;
    SRegionList regions;
    const int lineNumber;
    void _dump();
    static void dumpKeysAndValues(const SKeyValueList& v);
};
using SGroupPtr = std::shared_ptr<SGroup>;
using SGroupList = std::vector<SGroupPtr>;

// Eventually we will need more complex top level objects
#if 0
class SGlobal {
public:
    SKeyValueList values;
    SamplerSchema::KeysAndValuesPtr compiledValues;
};

class SControl {
public:
    SKeyValueList values;
    SamplerSchema::KeysAndValuesPtr compiledValues;
};
#endif

class SLex;
class SLexItem;
class SInstrument;
using SLexPtr = std::shared_ptr<SLex>;
using SLexItemPtr = std::shared_ptr<SLexItem>;
using SInstrumentPtr = std::shared_ptr<SInstrument>;

class SParse {
public:
    static std::string go(const std::string& s, SInstrumentPtr);
    static std::string goFile(const std::string& s, SInstrumentPtr);

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

    /* What is a "heading"?
     * it's somehting that can modify following regions: group, control, etc...
     * we will treat these all pretty much the same
     */
  //  static std::string matchGlobal(SGlobal&, SLexPtr);
 //   static std::string matchControl(SControl&, SLexPtr);
    static std::string matchRegions(SRegionList&, SLexPtr);
    static Result matchRegion(SRegionList&, SLexPtr);
 //   static std::string F(SGroupList&, SLexPtr lex);
    static std::string matchHeadings(SInstrumentPtr, SLexPtr lex);
  //  static Result matchGroup(SGroupList&, SLexPtr);
  // static std::string matchGroupsOrRegions(SGroupList&, SLexPtr lex);
    static std::string matchHeadingsOrRegions(SInstrumentPtr, SLexPtr lex);
    static Result matchHeading(SInstrumentPtr, SLexPtr);

    static std::string matchKeyValuePairs(SKeyValueList&, SLexPtr);
    static Result matchKeyValuePair(SKeyValueList&, SLexPtr);

    // return empty if it's not a tag
    static std::string getTagName(SLexItemPtr);
};