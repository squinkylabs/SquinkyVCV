#pragma once

#include <string>
#include <memory>
#include <vector>

namespace ci
{
    class KeysAndValues;
    using KeysAndValuesPtr = std::shared_ptr<KeysAndValues>;
};


class SKeyValuePair {
public:
    SKeyValuePair(const std::string& k ,const std::string& v) : key(k), value(v) {}
    SKeyValuePair() = default;
    std::string key;
    std::string value;
};
using SKeyValuePairPtr = std::shared_ptr<SKeyValuePair>; 
using SKeyValueList = std::vector<SKeyValuePairPtr>;

class SRegion {
public:
    SKeyValueList values;
    ci::KeysAndValuesPtr compiledValues;
};
using SRegionPtr = std::shared_ptr<SRegion>; 
using SRegionList = std::vector<SRegionPtr>;

// Groups have a list of values
// and they usually have children.
class SGroup {
public:
    SKeyValueList values;
    ci::KeysAndValuesPtr compiledValues;
    SRegionList regions;
};
using SGroupPtr = std::shared_ptr<SGroup>; 
using SGroupList = std::vector<SGroupPtr>;

// Eventually we will need more complex top level objects
class SGlobal {
public:
    SKeyValueList values;
    ci::KeysAndValuesPtr compiledValues;
};


class SLex;
class SLexItem;
class SInstrument;
using SLexPtr = std::shared_ptr<SLex>;
using SLexItemPtr = std::shared_ptr<SLexItem>;
using SInstrumentPtr = std::shared_ptr<SInstrument>;

class SParse
{
public:

  static std::string go(const std::string& s, SInstrumentPtr);
  static std::string goFile(const std::string& s, SInstrumentPtr);

private:

    class Result {
    public:
       
        std::string errorMessage;
        enum Res {
            ok,                 // matched
            no_match,           // finished matching
            error
        };
        Res res = Res::ok;
    };

    static std::string matchGlobal(SGlobal&, SLexPtr);
    static std::string matchRegions(SRegionList&, SLexPtr);
    static Result matchRegion(SRegionList&, SLexPtr);
    static std::string matchGroups(SGroupList& , SLexPtr lex);
     static Result matchGroup(SGroupList&, SLexPtr);
    static std::string matchGroupsOrRegions(SGroupList& , SLexPtr lex);

    static std::string matchKeyValuePairs(SKeyValueList&, SLexPtr);
    static Result matchKeyValuePair(SKeyValueList&, SLexPtr);

    // return empty if it's not a tag
    static std::string getTagName(SLexItemPtr);

};