
#include "SParse.h"
#include "SLex.h"

#include <assert.h>

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
        if (nextToken->itemType != SLexItem::Type::Tag) {
            printf("top level item not a tag\n");
            return nullptr;
        }
        SLexTag* tag = static_cast<SLexTag *>(nextToken.get());
        if (tag->tagName == "global") {
            lex->consume();
            theInstrument = parseGlobal(theInstrument, lex);
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
   //assert(false);
   //return nullptr;
    return inst;
}

  // return 0 = got one, 1 = all done, 2 = error
int SParse::matchRegion(SDataInstrumentPtr inst, SLexPtr lex) {

    // consume the open tag
    auto nextToken = lex->next();
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