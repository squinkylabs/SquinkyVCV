

#include "SParse.h"

#include <assert.h>

#include <fstream>
#include <set>
#include <streambuf>
#include <string>

#include "SInstrument.h"
#include "SLex.h"
#include "SqLog.h"
#include "SqStream.h"

// global for mem leak detection
int parseCount = 0;

/*
poassible way to parse non-standard control blocks:

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

#if 0
    std::string sError = matchControl(inst->control, lex);
    if (!sError.empty()) {
        return sError;
    }
    sError = matchGlobal(inst->global, lex);
    if (!sError.empty()) {
        return sError;
    }
#endif
    std::string sError = matchHeadingsOrRegions(inst, lex);
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

// try to make a list of groups. If not possible,
// make a dummy group and put the regions inside
std::string SParse::matchHeadingsOrRegions(SInstrumentPtr inst, SLexPtr lex) {
    SQINFO("matchHeadingsOrRegions");
    auto token = lex->next();
    if (!token) {
        SQINFO("leave early no tokens");
        return "";  // nothing left to match
    }
    if (getTagName(token) == "region") {
        SQINFO("matchHeadingsOrRegions is region 114");
        // OK, the first thing is a region. To let's put it in a group, and continue
        SGroupPtr fakeGroup = std::make_shared<SGroup>(token->lineNumber);
        inst->groups.push_back(fakeGroup);
        auto resultString = matchRegions(fakeGroup->regions, lex);
        if (!resultString.empty()) {
            SQINFO("120 match regions fail");
            return resultString;
        }
        SQINFO("matchHeadingsOrRegions will call matchHeadings");
        // now continue on adding more groups, if present
        return matchHeadingGroups(inst, lex);
    }
    // compare to groups? that doesn't seem right
    // SQWARN("why are we looking for tag name??");
    //  assert(false);
    //  if (getTagName(token) == "group") {
    //       return matchHeadings(inst, lex);
    //   }

    // does this check do anything??
    if (!getTagName(token).empty()) {
        SQINFO("calling matchHeadings, since not region");
        return matchHeadingGroups(inst, lex);
    }

    return "";
}

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

std::string SParse::matchRegions(SRegionList& regions, SLexPtr lex) {
    SQINFO("matchRegions size=%d", regions.size());
    for (bool done = false; !done;) {
        auto result = matchRegion(regions, lex);
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
    SQINFO("call isHeadingNanme on %s", s.c_str());
    return headingTags.find(s) != headingTags.end();
}

std::pair<SParse::Result, bool> SParse::matchSingleHeading(SInstrumentPtr inst, SLexPtr lex) {
    Result result;

    SQINFO(" SParse::matchSingleHeading");
    auto tok = lex->next();

    // if this cant match a heading, the give up
    if (!tok || !isHeadingName(getTagName(tok))) {
        if (!tok)
            SQINFO("matchSingleHeading exit early out of tokens ");
        else
            SQINFO("matchSingleHeading exit early on tag %s", getTagName(tok));

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
    SQINFO("matchSingleHeading got keys and values");

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
    bool matchedOne = false;
    for (bool done=false; !done; ) {
        std::pair<Result, bool> temp = matchSingleHeading(inst, lex);

        switch (temp.first.res) {
            case Result::ok:
                matchedOne = true;
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
                // if we match no headings, then we failed the match
                if (!matchedOne) {
                    return temp.first;
                }
                // if some matched, then that just means we are done matching
        }
    }

    // Getting here means we have successfully parsed at least one
    // heading, and we are done with them. Now we just need to round up the regions

    // and continue an get all the region children
    // TODO: copy all the data into the new group!!
    Result result;
    SGroupPtr newGroup = std::make_shared<SGroup>(inst->currentGroup.lineNumber);
    inst->groups.push_back(newGroup);
    assert(newGroup);
    std::string regionsError = matchRegions(newGroup->regions, lex);
    if (!regionsError.empty()) {
        result.res = Result::Res::error;
        result.errorMessage = regionsError;
    }

    return result;
}
// Matches headings followed by a region list
#if 0
SParse::Result SParse::matchHeadingGroup(SInstrumentPtr inst, SLexPtr lex) {
    SQINFO(" SParse::matchHeading");
    Result result;
    auto tok = lex->next();

    // if this cant match a heading, the give up
    if (!tok || !isHeadingName(getTagName(tok))) {
        result.res = Result::Res::no_match;
        if (!tok)
            SQINFO("match heading exit early out of tokens ");
        else
            SQINFO("match heading exit early on tag %s", getTagName(tok));

        return result;
    }

    const std::string tagName = getTagName(tok);
    // consume the [heading] tag
    lex->consume();

    // make a new group to hold this one. If the tag isn't for a
    // group, then we will copy it.
    SGroupPtr newGroup = std::make_shared<SGroup>(tok->lineNumber);

    // add all the key-values that belong to the group
    std::string s = matchKeyValuePairs(newGroup->values, lex);

    if (!s.empty()) {
        result.res = Result::Res::error;
        result.errorMessage = s;
    }

    // now stash all the key values where they really belong
    SKeyValueList* dest = nullptr;

    if (tagName == "global") {
        dest = &inst->currentGlobal;
    } else if (tagName == "control") {
        dest = &inst->currentControl;
    } else if (tagName == "master") {
        dest = &inst->currentMaster;
    }

    // if it's a "global" heading, then copy the values for later.
    // if it's a real group, then save it off and terminate everything
    if (dest) {
        *dest = newGroup->values;
        newGroup = nullptr;
        SQINFO("match headling  non group %s", tagName.c_str());

        auto tok = lex->next();
        if (!tok || getTagName(tok) != "region") {
            SQINFO("match heading leaving early = not group or region");
            return result;
        }
    }

    // here it is either a region or a group

    // if it's a real group, then we need to combine all the temporary data into this group
    // (Or not, depending on how we go here)

    if (newGroup) {
        // now we need to coalesce all the global props into this group
        SQINFO("!!242 finish");
        inst->groups.push_back(newGroup);
    }

    // this will make our parser assume that the very last heading in a block
    // is a group. That's an ok assumption, but we could easily change it...
    if (newGroup) {
        SQINFO("continue on to reading the region children");

        // and continue an get all the region children
        assert(newGroup);
        std::string regionsError = matchRegions(newGroup->regions, lex);
        if (!regionsError.empty()) {
            result.res = Result::Res::error;
            result.errorMessage = regionsError;
        }
    }
    return result;
}
#endif

SParse::Result SParse::matchRegion(SRegionList& regions, SLexPtr lex) {
    SQINFO("matchRegion regions size = %d", regions.size());
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

    SRegionPtr newRegion = std::make_shared<SRegion>(tag->lineNumber);
    regions.push_back(newRegion);

    std::string s = matchKeyValuePairs(newRegion->values, lex);

    if (!s.empty()) {
        result.res = Result::Res::error;
        result.errorMessage = s;
    }
    return result;
}

#if 0
std::string SParse::matchGlobal(SGlobal& g, SLexPtr lex) {
    auto token = lex->next();
    std::string sError;
    if (getTagName(token) == "global") {
        lex->consume();
        sError = matchKeyValuePairs(g.values, lex);
    }

    return sError;
}

std::string SParse::matchControl(SControl& c, SLexPtr lex) {
    auto token = lex->next();
    std::string sError;
    if (getTagName(token) == "control") {
        lex->consume();
        sError = matchKeyValuePairs(c.values, lex);
    }

    return sError;
}
#endif

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