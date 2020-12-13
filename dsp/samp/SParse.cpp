
#include "SParse.h"
#include "SLex.h"

#include <assert.h>
#include <string>
#include <fstream>
#include <streambuf>

std::string SParse::goFile(const std::string& sPath, SInstrumentPtr inst) {
    
    FILE* fp = nullptr;
     fopen_s(&fp, sPath.c_str(), "r");
    if (fp) {
        fclose(fp);
    }
    std::ifstream t(sPath);
    if (!t.good()) {
        printf("can't open file\n");
        return nullptr;
    }
    std::string str((std::istreambuf_iterator<char>(t)),
                 std::istreambuf_iterator<char>());
    if (str.empty()) {
        return nullptr;
    }
    return go(str, inst);
}

std::string SParse::go(const std::string& s, SInstrumentPtr inst) {

    SLexPtr lex = SLex::go(s);
    if (!lex) {
        printf("lexer failed\n");
        return "";
    }
    lex->_dump();

    std::string sError = matchGlobal(inst->global, lex);
    if (!sError.empty()) {
        return sError;
    }
    sError = matchGroupsOrRegions(inst->groups, lex);
    if (!sError.empty()) {
        return sError;
    }
    if (lex->next() != nullptr) {
        printf("extra tok index %d\n", lex->_index());
        return "extra tokens";
    }
    if (inst->groups.empty()) {
        return "no groups or regions";
    }
    return sError;
}

// try to make a list of groups. If not possible,
// make a dummy group and put the regions inside
std::string SParse::matchGroupsOrRegions(SGroupList& groups, SLexPtr lex) {
    auto token = lex->next();
    if (!token) {
        return "";          // nothing left to match
    }
    if (getTagName(token) == "region") {
          // OK, the first thing is a region. To let's put it in a group, and continue
        SGroupPtr fakeGroup = std::make_shared<SGroup>();
        groups.push_back(fakeGroup);
        auto resultString = matchRegions(fakeGroup->regions, lex);
        if (!resultString.empty()) {
            return resultString;
        }
        // now continue on adding more groups, if present
        return matchGroups(groups, lex);
    }
    if (getTagName(token) == "group") {
         return matchGroups(groups, lex);
    }
   
    return "";
}

std::string SParse::matchGroups(SGroupList& groups, SLexPtr lex) {
     for (bool done = false; !done; ) {
        auto result = matchGroup(groups, lex);
        if (result.res == Result::error) {
            return result.errorMessage;
        }
        done = result.res == Result::no_match;
    }
    return "";
}

std::string SParse::matchRegions(SRegionList& regions, SLexPtr lex) {
    for (bool done = false; !done; ) {
        auto result = matchRegion(regions, lex);
        if (result.res == Result::error) {
            return result.errorMessage;
        }
        done = result.res == Result::no_match;
    }
    return "";
}

SParse::Result SParse::matchGroup(SGroupList& groups, SLexPtr lex) {
    Result result;
    auto tok = lex->next();
    if (!tok || (getTagName(tok) != "group")) {
        result.res = Result::Res::no_match;
        return result;
    }

    // consume the <group> tag
    lex->consume();

    // make a new group to hold this one, and put it into the groups
    SGroupPtr newGroup = std::make_shared<SGroup>();
    groups.push_back(newGroup);

    // add all the key-values that belong to the group
    std::string s = matchKeyValuePairs(newGroup->values, lex);
   
    if (!s.empty()) {
        result.res = Result::Res::error;
        result.errorMessage = s;
    }

    std::string regionsError = matchRegions(newGroup->regions, lex);
    if (!regionsError.empty()) {
        result.res = Result::Res::error;
        result.errorMessage = regionsError;
    }
    return result;
}

SParse::Result SParse::matchRegion(SRegionList& regions, SLexPtr lex) {
    Result result;
    auto tok = lex->next();
    if (!tok || (getTagName(tok) != "region")) {
        result.res = Result::Res::no_match;
        return result;
    }

    // consume the <region> tag
    lex->consume();

    // make a new region to hold this one, and put it into the group
    SRegionPtr newRegion = std::make_shared<SRegion>();
    regions.push_back(newRegion);

    std::string s = matchKeyValuePairs(newRegion->values, lex);
   
    if (!s.empty()) {
        result.res = Result::Res::error;
        result.errorMessage = s;
    }
    return result;

}

#if 0
SParse::Result SParse::matchRegions(SRegionList& regions, SLexPtr lex) {
    Result result;

    // is the first potential region empty?
    auto tok = lex->next();
    if (!tok) {
        result.res = Result::Res::no_match;
        return result;
    }
    if (getTagName(tok) != "region") {
        result.res = Result::Res::error;
        result.errorMessage = "first potential region not resion";
    }
    // Ok, here we know we have a region
    assert(false);
    return result;
}
#endif

std::string SParse::matchGlobal(SGlobal& g, SLexPtr lex) {
    auto token = lex->next();
    std::string sError;
    if (getTagName(token) == "global") {
        lex->consume();
        sError = matchKeyValuePairs(g.values, lex);
    }
  
    return sError;
}

std::string SParse::matchKeyValuePairs(SKeyValueList& values, SLexPtr lex) {
    for (bool done=false; !done; ) {
        auto result = matchKeyValuePair(values, lex);
        if (result.res == Result::error) {
            return result.errorMessage;
        }
        done = result.res == Result::no_match;
    }
    
    return "";
}

SParse::Result SParse::matchKeyValuePair(SKeyValueList& values, SLexPtr lex) {
    auto keyToken = lex->next();
    Result result;

    // if all done, or no more pairs, then leave
    if (!keyToken || (keyToken->itemType != SLexItem::Type::Identifier)) {
        result.res = Result::no_match;
        return result;
    }

    SLexIdentifier* pid =  static_cast<SLexIdentifier *>(keyToken.get());
    SKeyValuePairPtr thePair = std::make_shared<SKeyValuePair>();
    thePair->key = pid->idName;
    lex->consume();

    keyToken = lex->next();
    if (keyToken->itemType != SLexItem::Type::Equal) {
        result.errorMessage = "= in kvp missing =";
        result.res = Result::error;
        return result;
    }
    lex->consume();

    keyToken = lex->next();
    if (keyToken->itemType != SLexItem::Type::Identifier) {
        result.errorMessage = "value in kvp not id";
        result.res = Result::error;
        return result;
    }
    lex->consume();
    pid =  static_cast<SLexIdentifier *>(keyToken.get());
    thePair->value = pid->idName;

    values.push_back(thePair);
    return result;
}


std::string SParse::getTagName(SLexItemPtr item) {
    // maybe shouldn't call this with null ptr??
    if (!item) {
        return "";
    }
    if (item->itemType != SLexItem::Type::Tag) {
        return "";
    }
    SLexTag* tag = static_cast<SLexTag *>(item.get());
    return tag->tagName;
}

#if 0 // old stuff, version 2

SDataInstrumentPtr SParse::matchStart(SLexPtr lex) {
    auto nextToken = lex->next();

    SDataInstrumentPtr inst;
    // at the start there must be at least one tag
    if (nextToken->itemType == SLexItem::Type::Tag) {
        inst = finishMatchStart(lex);
        if (!inst) {
            printf("match start failed\n");
            return nullptr;
        }
    }

    return matchRegionList(inst, lex);
}

bool SParse::isLegalTopLevelGlobalName(const std::string& name) {
    if (name == "global") {
        return true;
    }
    //assert(Name == "group" || name == "region");
    return false;
}

bool SParse::isLegalRegionListName(const std::string& name) {
    return (name == "region" || name == "group");
}

// we want to read in all the tags that are valid top level tags.
// leaving behind the regions list
SDataInstrumentPtr SParse::finishMatchStart(SLexPtr lex) {
    SDataInstrumentPtr theInstrument = std::make_shared<SDataInstrument>();
    bool readingGlobals = true;
    while(readingGlobals) {
        auto nextToken = lex->next();
        if (!nextToken) {
            printf("match start early exit (NOT ERROR)\n");
            return theInstrument;
        }

        if (nextToken->itemType != SLexItem::Type::Tag) {
            printf("error: top level item not a tag\n");
            return nullptr;
        }

        SLexTag* tag = static_cast<SLexTag *>(nextToken.get());
        if (isLegalTopLevelGlobalName(tag->tagName)) {
            assert(false);
        } else {
            //assert(false);
            // when we hit a tag that's not global, we stop.
            // will parse that next
            return theInstrument;
        }

        assert(theInstrument);
    }
    return theInstrument;
}

SDataInstrumentPtr SParse::matchRegionList(SDataInstrumentPtr inst, SLexPtr lex) {
    auto nextToken = lex->next();
    if (nextToken->itemType != SLexItem::Type::Tag) {
        printf("error: region list has invalid start item\n");
        return nullptr;
    }
    SLexTag* tag = static_cast<SLexTag *>(nextToken.get());
    if (!isLegalRegionListName(tag->tagName)) {
        printf("error: region list may not contain %s\n", tag->tagName.c_str());
    }

    lex->consume();         // ok, we accept this start tag
   // inst = matchKeyValuePairs(int, lex);
    inst = matchRegionListItems(inst, lex);
    return inst;
}

// TODO: should pass in empty list!
SDataInstrumentPtr SParse::matchRegionListItems(SDataInstrumentPtr inst, SLexPtr lex) {
    auto nextToken = lex->next();
    if (nextToken->itemType != SLexItem::Type::Tag) {
        // if it's not a tag, it must be a key/value pair.
        auto kvp = matchKeyValuePair(lex);
        assert(false);
    }
    else {
        assert(false);
    }
    assert(false);
    return nullptr;
}

SKeyValuePairPtr SParse::matchKeyValuePair(SLexPtr lex) {
    auto keyToken = lex->next();
    if (keyToken->itemType != SLexItem::Type::Identifier) {
        printf("key in kvp not id\n");
        return nullptr;
    }
    SLexIdentifier* pid =  static_cast<SLexIdentifier *>(keyToken.get());
    SKeyValuePairPtr thePair = std::make_shared<SKeyValuePair>();
    thePair->key = pid->idName;

    keyToken = lex->next();
    if (keyToken->itemType != SLexItem::Type::Equal) {
        printf("= in kvp missing\n");
        return nullptr;
    }

    keyToken = lex->next();
    if (keyToken->itemType != SLexItem::Type::Identifier) {
        printf("value in kvp not id\n");
        return nullptr;
    }
    pid =  static_cast<SLexIdentifier *>(keyToken.get());
    thePair->value = pid->idName;

    return thePair;
}
#endif

#if 0   // OLD stuff, version 1
SDataInstrumentPtr SParse::matchStart(SLexPtr lex) {
    auto nextToken = lex->next();

    // at the start there must be at least one tag
    if (nextToken->itemType == SLexItem::Type::Tag) {
        auto inst = finishMatchStart(lex);
        if (!inst) {
            printf("match start failed\n");
            return nullptr;
        }
        // probably over simplistic to assume only one group
        return matchRegions(inst, lex);
    }
    printf("instrument does not start with any tags\n");
    return nullptr;
}

SDataInstrumentPtr SParse::finishMatchStart(SLexPtr lex) {
    SDataInstrumentPtr theInstrument = std::make_shared<SDataInstrument>();
    bool readingGlobals = true;
    while(readingGlobals) {
        auto nextToken = lex->next();
        if (!nextToken) {
            printf("match start early exit\n");
            return theInstrument;
        }
        if (nextToken->itemType != SLexItem::Type::Tag) {
            printf("top level item not a tag\n");
            return nullptr;
        }
        SLexTag* tag = static_cast<SLexTag *>(nextToken.get());
        if (tag->tagName == "global") {
            lex->consume();
            theInstrument = parseGlobal(theInstrument, lex);
        } else if (tag->tagName == "group") {
            assert(false);
        } else if (tag->tagName == "region") {
            readingGlobals = false;
        } else {
            assert(false);
        }     
        
        if (!theInstrument) {
            return nullptr;
        }

    }
    return theInstrument;
}

  SDataInstrumentPtr SParse::parseGlobal(SDataInstrumentPtr inst, SLexPtr lex) {
      bool parsingGlobal = true;
      while(parsingGlobal) {
          printf("we aren't really parsing global\n");
           auto tok = lex->next();
           if (!tok) {
               return inst;
           }
           if (tok->itemType == SLexItem::Type::Tag) {
               return inst;
           }
           lex->consume();          // just eat everything until we are done.

      }
      assert(false);
      return nullptr;
  }

SDataInstrumentPtr SParse::matchRegions(SDataInstrumentPtr inst, SLexPtr lex) {
    bool readingRegions = true;
    while(readingRegions) {
        int x = matchRegion(inst, lex);
        switch(x) {
            case 2:
                return nullptr;
                break;
            case 0:
                break;
            case 1:
               readingRegions = false;
               break;
            default:
                assert(false);
                readingRegions = false;
        }
    }

    // we read all the regions;
    auto remainingTok = lex->next();
    if (remainingTok) {
        printf("extra tokens after region\n");
        return nullptr;
    }

    return inst;
}

  // return 0 = got one, 1 = all done, 2 = error
int SParse::matchRegion(SDataInstrumentPtr inst, SLexPtr lex) {

    // consume the open tag
    auto nextToken = lex->next();
    if (!nextToken) {
        return 1;
    }
    if (nextToken->itemType != SLexItem::Type::Tag) {
        assert(false);
        return 2;
    }
    SLexTag* tag = static_cast<SLexTag *>(nextToken.get());
    if (tag->tagName != "region") {
        assert(false);
        return 2;
    }
    lex->consume();

    // read all the region values
    bool readingValues = true;
    while(readingValues) {

        // read key
        nextToken = lex->next();
        if (!nextToken) {
            readingValues = false;
            return 1;
        }
        if (nextToken->itemType != SLexItem::Type::Identifier) {
            printf("malformed region data\n");
            return 2;
        }
        SLexIdentifier* pid = static_cast<SLexIdentifier *>(nextToken.get());
        std::string key = pid->idName;
        lex->consume();

        // read =
        nextToken = lex->next();
        if (!nextToken) {
            readingValues = false;
            return 1;
        }

        if (nextToken->itemType != SLexItem::Type::Equal) {
            printf("malformed region data2\n");
            return 2;
        }
        lex->consume();

        // read value
        nextToken = lex->next();
        if (!nextToken) {
            readingValues = false;
            return 1;
        }
        if (nextToken->itemType != SLexItem::Type::Identifier) {
            printf("malformed region data\n");
            return 2;
        }
        pid = static_cast<SLexIdentifier*>(nextToken.get());
        std::string value = pid->idName;
        lex->consume();


        // now we shoud decode an do something with it.
    }

    nextToken = lex->next();
    if (!nextToken) {
        readingValues = false;
        return 1;
    }

    assert(false);
    return 2;
}
#endif