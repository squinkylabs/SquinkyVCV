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


// lists are made up or groups and regions
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


// An entire instrument.
class SDataInstrument {
public:
    using ListOfThings = std::vector<SRegionOrGroupInstrumentPtr>;

  //  SGroupInstrument global;
    ListOfThings topLevelThings;
    ListOfThings regionList;
};
using SDataInstrumentPtr = std::shared_ptr<SDataInstrument>;


class SLex;
using SLexPtr = std::shared_ptr<SLex>;

class SParse
{
public:
    static SDataInstrumentPtr go(const std::string& s);
    static SDataInstrumentPtr goFile(const std::string& sPath);

private:
    static SDataInstrumentPtr matchStart(SLexPtr);
    // must be called with next item is a tag
    static SDataInstrumentPtr finishMatchStart(SLexPtr);

    #if 0   // TODO

    static SDataInstrumentPtr parseGlobal(SDataInstrumentPtr, SLexPtr);

  //  static SDataInstrumentPtr matchRegions(SDataInstrumentPtr, SLexPtr);

    // Reads all the regions and puts them in destination.
    matchRegions(SDataInstrument::ListOfThings destination, SLexPtr lex);
    // return 0 = got one, 1 = all done, 2 = error
    static int matchRegion(SDataInstrumentPtr, SLexPtr);
#endif 

    static bool isLegalTopLevelGlobalName(const std::string& name);
    static bool isLegalRegionListName(const std::string& name);
    static SDataInstrumentPtr matchRegionList(SDataInstrumentPtr inst, SLexPtr lex);
    static SDataInstrumentPtr matchRegionListItems(SDataInstrumentPtr inst, SLexPtr lex);
    static SKeyValuePairPtr matchKeyValuePair(SLexPtr lex);



};