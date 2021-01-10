
#include "SParse.h"

#include <assert.h>

#include <fstream>
#include <streambuf>
#include <string>

#include "SInstrument.h"
#include "SLex.h"

// globals for mem leak detection
int parseCount = 0;

std::string SParse::goFile(const std::string& sPath, SInstrumentPtr inst) {
#if 0
    FILE* fp = nullptr;
    fopen_s(&fp, sPath.c_str(), "r");
    if (fp) {
        fclose(fp);
    }
#endif
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
    // lex->_dump();

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
        return "";  // nothing left to match
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
    for (bool done = false; !done;) {
        auto result = matchGroup(groups, lex);
        if (result.res == Result::error) {
            return result.errorMessage;
        }
        done = result.res == Result::no_match;
    }
    return "";
}

std::string SParse::matchRegions(SRegionList& regions, SLexPtr lex) {
    for (bool done = false; !done;) {
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

    SLexItem* item = tok.get();
    SLexTag* tag = static_cast<SLexTag*>(item);

    // consume the <region> tag
    lex->consume();

    // make a new region to hold this one, and put it into the group

    SRegionPtr newRegion = std::make_shared<SRegion>(tag->lineNumber);
    regions.push_back(newRegion);

    std::string s = matchKeyValuePairs(newRegion->values, lex);

    if (!s.empty()) {
        result.res = Result::Res::error;
        result.errorMessage = s;
    }
    return result;
}

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
    if (keyToken->itemType != SLexItem::Type::Equal) {

        result.errorMessage = "= in kvp missing equal sign at file line# " + keyToken->lineNumberAsString();
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
void SGroup::dumpKeysAndValues(const SKeyValueList& v) {
    for (auto x : v) {
        SKeyValuePairPtr kvp = x;
        printf("%s=%s\n", kvp->key.c_str(), kvp->value.c_str());
    }
}