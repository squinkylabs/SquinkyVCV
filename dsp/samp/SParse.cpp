
#include "SParse.h"


#include "FilePath.h"
#include "SInstrument.h"
#include "SLex.h"
#include "SqLog.h"
#include "SqStream.h"

#include <assert.h>
#include <fstream>
#include <set>
#include <streambuf>
#include <string>

// global for mem leak detection
int parseCount = 0;

/*
possible way to parse non-standard control blocks:

do it mostly in the parser.
always keep a "current" global, control, midi, master
when we match "group" : match any order of "non region" things
when we make a group, put all attributes from the other "things" into it

Do we take the   SControl control and SGlobal global out of SInstrument? if so we need some persistent
parse state to hold onto... I guess we could continue to use sINst, but it's a bit of a kludge...

only non-parser thing:
    put the paths together when we make the compiled regions.
    eliminate the getPath accessor from ci.
*/

std::string SParse::goFile(const FilePath& filePath, SInstrumentPtr inst) {
    std::ifstream t(filePath.toString());
    if (!t.good()) {
      //  printf("can't open file\n");
        return "can't open source file: " + filePath.toString();
    }
    std::string str((std::istreambuf_iterator<char>(t)),
                    std::istreambuf_iterator<char>());
    if (str.empty()) {
        return "file empty: " + filePath.toString();
    }
    return goCommon(str, inst, &filePath);
}

std::string SParse::go(const std::string& s, SInstrumentPtr inst) {
    return goCommon(s, inst, nullptr);
}

std::string SParse::goCommon(const std::string& sContent, SInstrumentPtr outParsedInstrument, const FilePath* fullPathToSFZ) {
    std::string lexError;
    SLexPtr lex = SLex::go(sContent, &lexError, 0, fullPathToSFZ);
    if (!lex) {
        assert(!lexError.empty());
        return lexError;
    }

    std::string sError = matchHeadingGroups(outParsedInstrument, lex);
    if (!sError.empty()) {
        return sError;
    }
    if (lex->next() != nullptr) {
        auto item = lex->next();
        auto type = item->itemType;
        auto lineNumber = item->lineNumber;
        SqStream errorStream;
        errorStream.add("extra tok line number ");
        errorStream.add(int(lineNumber));
        errorStream.add(" type= ");
        errorStream.add(int(type));
        errorStream.add(" index=");
        errorStream.add(lex->_index());
        //printf("extra tok line number %d type = %d index=%d\n", int(lineNumber), int(type), lex->_index());
        if (type == SLexItem::Type::Identifier) {
            SLexIdentifier* id = static_cast<SLexIdentifier*>(item.get());
            // printf("id name is %s\n", id->idName.c_str());
            errorStream.add(" id name is ");
            errorStream.add(id->idName);
        }
        return errorStream.str();
    }
    if (outParsedInstrument->groups.empty()) {
        return "no groups or regions";
    }
    return sError;
}

#if 0
std::string SParse::go(const std::string& s, SInstrumentPtr inst) {
    std::string lexError;
    SLexPtr lex = SLex::go(s, &lexError);
    if (!lex) {
        assert(!lexError.empty());
        return lexError;
    }

    std::string sError = matchHeadingGroups(inst, lex);
    if (!sError.empty()) {
        return sError;
    }
    if (lex->next() != nullptr) {
        auto item = lex->next();
        auto type = item->itemType;
        auto lineNumber = item->lineNumber;
        SqStream errorStream;
        errorStream.add("extra tok line number ");
        errorStream.add(int(lineNumber));
        errorStream.add(" type= ");
        errorStream.add(int(type));
        errorStream.add(" index=");
        errorStream.add(lex->_index());
        //printf("extra tok line number %d type = %d index=%d\n", int(lineNumber), int(type), lex->_index());
        if (type == SLexItem::Type::Identifier) {
            SLexIdentifier* id = static_cast<SLexIdentifier*>(item.get());
            // printf("id name is %s\n", id->idName.c_str());
            errorStream.add(" id name is ");
            errorStream.add(id->idName);
        }
        return errorStream.str();
    }
    if (inst->groups.empty()) {
        return "no groups or regions";
    }
    return sError;
}
#endif

std::string SParse::matchHeadingGroups(SInstrumentPtr inst, SLexPtr lex) {
    for (bool done = false; !done;) {
        auto result = matchHeadingGroup(inst, lex);
        if (result.res == Result::error) {
            return result.errorMessage;
        }
        done = result.res == Result::no_match;
    }
    return "";
}

std::string SParse::matchRegions(SRegionList& regions, SLexPtr lex, const SHeading& controlBlock) {
    for (bool done = false; !done;) {
        auto result = matchRegion(regions, lex, controlBlock);
        if (result.res == Result::error) {
            return result.errorMessage;
        }
        done = result.res == Result::no_match;
    }
    return "";
}

static std::set<std::string> headingTags = {
    {"group"},
    {"global"},
    {"control"},
    {"master"}};

static bool isHeadingName(const std::string& s) {
    return headingTags.find(s) != headingTags.end();
}

std::pair<SParse::Result, bool> SParse::matchSingleHeading(SInstrumentPtr inst, SLexPtr lex) {
    Result result;

    auto tok = lex->next();

    // if this cant match a heading, the give up
    if (!tok || !isHeadingName(getTagName(tok))) {
        result.res = Result::no_match;
        return std::make_pair(result, false);
    }

    // ok, here we matched a heading. Remember the name
    // and consume the [heading] token.
    const std::string tagName = getTagName(tok);
    lex->consume();

    // now extract out all the keys and values for this heading
    SKeyValueList keysAndValues;
    std::string s = matchKeyValuePairs(keysAndValues, lex);

    if (!s.empty()) {
        result.res = Result::Res::error;
        result.errorMessage = s;
        return std::make_pair(result, false);
    }

    // now stash all the key values where they really belong
    SHeading* dest = nullptr;
    bool isGroup = false;
    if (tagName == "global") {
        dest = &inst->global;
    } else if (tagName == "control") {
        dest = &inst->currentControl;
    } else if (tagName == "master") {
        dest = &inst->master;
    } else if (tagName == "group") {
        dest = &inst->currentGroup;
        isGroup = true;
    }


    dest->values = std::move(keysAndValues);
    return std::make_pair(result, isGroup);
}

// matches a series of headings, followed by a series of regions
// (for now?) assume each series of headings ends with a group

SParse::Result SParse::matchHeadingGroup(SInstrumentPtr inst, SLexPtr lex) {
    bool matchedOneHeading = false;
    for (bool done=false; !done; ) {
        std::pair<Result, bool> temp = matchSingleHeading(inst, lex);

        switch (temp.first.res) {
            case Result::ok:
                matchedOneHeading = true;
                if (temp.second) {
                    // here we assume that the last heading will be a group
                    // maybe we should relax this so we can use this in the non-group case?
                    done = true;
                }
                break;
            case Result::error:
                return temp.first;
                break;
            case Result::no_match:
                // if we match no headings, then we don't look for more,
                // but it's not an error. region with no headings is ok
                done = true;
        }
    }

    // Getting here means we have successfully parsed at least one
    // heading, and we are done with them. Now we just need to round up the regions

    // and continue an get all the region children
    // TODO: copy all the data into the new group!!

    Result result;
    SGroupPtr newGroup = std::make_shared<SGroup>(inst->currentGroup.lineNumber);
    newGroup->values = inst->currentGroup.values;
    inst->currentGroup.values.clear();

    assert(newGroup);
    // TODO: clean out control when appropriate
    std::string regionsError = matchRegions(newGroup->regions, lex, inst->currentControl);
    if (!regionsError.empty()) {
        result.res = Result::Res::error;
        result.errorMessage = regionsError;
    }

    if (!matchedOneHeading && newGroup->regions.empty()) {
        result.res = Result::no_match;
        // SQINFO("matchHeadingGroup found no match");
    }

    // if we found regions, then this group is "real"
    if (result.res == Result::ok) {
        inst->groups.push_back(newGroup);
    }

    return result;
}

SParse::Result SParse::matchRegion(SRegionList& regions, SLexPtr lex, const SHeading& controlBlock) {
    // SQINFO("matchRegion regions size = %d", regions.size());
    Result result;
    auto tok = lex->next();
    if (!tok || (getTagName(tok) != "region")) {
        result.res = Result::Res::no_match;
        return result;
    }

    SLexItem* item = tok.get();
    SLexTag* tag = static_cast<SLexTag*>(item);

    // consume the <region> tag
    lex->consume();

    // does this invalidate/ free tok??

    // make a new region to hold this one, and put it into the group

    SRegionPtr newRegion = std::make_shared<SRegion>(tag->lineNumber, controlBlock);
    regions.push_back(newRegion);

    std::string s = matchKeyValuePairs(newRegion->values, lex);

    if (!s.empty()) {
        result.res = Result::Res::error;
        result.errorMessage = s;
    }
    return result;
}

std::string SParse::matchKeyValuePairs(SKeyValueList& values, SLexPtr lex) {
    for (bool done = false; !done;) {
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

    SLexIdentifier* pid = static_cast<SLexIdentifier*>(keyToken.get());
    SKeyValuePairPtr thePair = std::make_shared<SKeyValuePair>();
    thePair->key = pid->idName;
    lex->consume();

    keyToken = lex->next();
    if (!keyToken) {
        result.errorMessage = "= unexpected end of tokens" ;
        result.res = Result::error;
        return result;
    }
    if (keyToken->itemType != SLexItem::Type::Equal) {
        result.errorMessage = "= in kvp missing equal sign at file line# " + keyToken->lineNumberAsString();
        result.res = Result::error;
        return result;
    }
    lex->consume();

    keyToken = lex->next();
    if (keyToken->itemType != SLexItem::Type::Identifier) {
        result.errorMessage = "value in kvp is not id. key=" + thePair->key + " line# " + keyToken->lineNumberAsString();
        result.res = Result::error;
        return result;
    }
    lex->consume();
    pid = static_cast<SLexIdentifier*>(keyToken.get());
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
    SLexTag* tag = static_cast<SLexTag*>(item.get());
    return tag->tagName;
}

void SGroup::_dump() {
    printf("dumping group ----\n");
    dumpKeysAndValues(values);
    printf("done dumping group ----\n");
}

void SRegion::_dump() {
    printf("dumping region ----\n");
    SGroup::dumpKeysAndValues(values);
    printf("done dumping region ----\n");
}
void SHeading::dumpKeysAndValues(const SKeyValueList& v) {
    for (auto x : v) {
        SKeyValuePairPtr kvp = x;
        printf("%s=%s\n", kvp->key.c_str(), kvp->value.c_str());
    }
}