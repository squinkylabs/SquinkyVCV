#pragma once

#include <string>
#include <memory>
#include <vector>


class SKeyValuePair {
public:
    std::string key;
    std::string value;
};
using SKeyValuePairPtr = std::shared_ptr<SKeyValuePair>; 
using SKeyValueList = std::vector<SKeyValuePairPtr>;

class SRegion {
public:
    SKeyValueList values;
};
using SRegionPtr = std::shared_ptr<SRegion>; 
using SRegionList = std::vector<SRegionPtr>;

// Groups have a list of values
// and they usually have children.
class SGroup {
public:
   SRegionList regions;
};
using SGroupPtr = std::shared_ptr<SRegion>; 
using SGroupList = std::vector<SRegionPtr>;

// Eventually we will need more complex top level objects
class SGlobal {
public:
    SKeyValueList values;
};


// lists are made up or groups and regions
#if 0
class SRegionOrGroupInstrument {
public:
    std::vector<SKeyValuePair>  values;
};

using SRegionOrGroupInstrumentPtr = std::shared_ptr<SRegionOrGroupInstrument>;
using SListOfThings = std::vector<SRegionOrGroupInstrumentPtr>;



// groups are things that can be in lists,
// they have a list of values
// and they may have children.
class SGroupInstrument : public SRegionOrGroupInstrument {
public:
    SListOfThings children;
};

// regions may be in lists.
// they have a list of vaules
class SRegionInstrument : public SRegionOrGroupInstrument {
};
#endif

// An entire instrument.
class SInstrument {
public:
    SGlobal global;

    // Even if there are no groups, we make a dummy one so that data is nicer.
    SGroupList groups;

};
using SInstrumentPtr = std::shared_ptr<SInstrument>;



class SLex;
class SLexItem;
using SLexPtr = std::shared_ptr<SLex>;
using SLexItemPtr = std::shared_ptr<SLexItem>;

class SParse
{
public:

  // static SInstrumentPtr go(const std::string& s);
  //  static SInstrumentPtr goFile(const std::string& sPath);
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
    static std::string matchKeyValuePairs(SKeyValueList&, SLexPtr);
    static Result matchKeyValuePair(SKeyValueList&, SLexPtr);

    // return empty if it's not a tag
    static std::string getTagName(SLexItemPtr);
#if 0
    static SDataInstrumentPtr matchStart(SLexPtr);
    // must be called with next item is a tag
    static SDataInstrumentPtr finishMatchStart(SLexPtr);


    static bool isLegalTopLevelGlobalName(const std::string& name);
    static bool isLegalRegionListName(const std::string& name);
    static SDataInstrumentPtr matchRegionList(SDataInstrumentPtr inst, SLexPtr lex);
    static SDataInstrumentPtr matchRegionListItems(SDataInstrumentPtr inst, SLexPtr lex);
    static SKeyValuePairPtr matchKeyValuePair(SLexPtr lex);

#endif

};