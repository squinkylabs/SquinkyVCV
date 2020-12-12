
#include "SParse.h"
#include "SLex.h"

#include <assert.h>
#include <string>
#include <fstream>
#include <streambuf>

SDataInstrumentPtr SParse::goFile(const std::string& sPath) {
    
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
    return go(str);
}

SDataInstrumentPtr SParse::go(const std::string& s) {

    SLexPtr lex = SLex::go(s);
    if (!lex) {
        printf("lexer failed\n");
        return nullptr;
    }
    return matchStart(lex);
}

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
#if 0
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
#endif

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

#if 0
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