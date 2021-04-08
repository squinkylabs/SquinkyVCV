
#include "SParse.h"

#include <assert.h>

#include "FilePath.h"
#include "SInstrument.h"
#include "SLex.h"
#include "SqLog.h"
#include "SqStream.h"
#include "share/windows_unicode_filenames.h"

// global for mem leak detection
int parseCount = 0;

#if defined(ARCH_WIN)

FILE* SParse::openFile(const FilePath& fp) {
    // SQINFO("in win wide version open file");
    flac_set_utf8_filenames(true);
    return flac_internal_fopen_utf8(fp.toString().c_str(), "r");
}

#else

FILE* SParse::openFile(const FilePath& fp) {
    return fopen(fp.toString().c_str(), "r");
}

#endif

std::string SParse::readFileIntoString(FILE* fp) {
    // SQINFO("read f %p", fp );
    if (fseek(fp, 0, SEEK_END) < 0)
        return "";

    const long size = ftell(fp);
    if (size < 0)
        return "";

    if (fseek(fp, 0, SEEK_SET) < 0)
        return "";

    std::string res;
    res.resize(size);

    const size_t numRead = fread(const_cast<char*>(res.data()), 1, size, fp);
    // SQINFO("requested: %d, read %d", size, numRead);
    if (numRead != size) {
        res.resize(numRead);
    }
    return res;
}

std::string SParse::goFile(const FilePath& filePath, SInstrumentPtr inst) {
    // SQINFO("in Parse::go file with %s", filePath.toString().c_str());
    //  FILE* fp = fopen(filePath.toString().c_str(), "r");
    FILE* fp = openFile(filePath);
    //SQINFO("ret %p\n", fp);
    if (!fp) {
        // SQINFO("Parse::go canot open sfz");
        return "can't open " + filePath.toString();
    }
    std::string sContent = readFileIntoString(fp);
    // SQINFO("read content: %s", sContent.c_str());
    fclose(fp);
    return goCommon(sContent, inst, &filePath);
}

std::string SParse::go(const std::string& s, SInstrumentPtr inst) {
    return goCommon(s, inst, nullptr);
}

static std::string filter(const std::string& sInput) {
    std::string ret;
    for (char c : sInput) {
        if (c != '\r') {
            ret.push_back(c);
        }
    }
    return ret;
}

std::string SParse::goCommon(const std::string& sContentIn, SInstrumentPtr outParsedInstrument, const FilePath* fullPathToSFZ) {
    std::string sContent = filter(sContentIn);
    std::string lexError;
    SLexPtr lex = SLex::go(sContent, &lexError, 0, fullPathToSFZ);
    if (!lex) {
        assert(!lexError.empty());
        return lexError;
    }
   // SQINFO("here is lex output we will parse");
   // lex->_dump();

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
        if (type == SLexItem::Type::Tag) {
            auto tag = std::static_pointer_cast<SLexTag>(item);
            SQINFO("extra tok = %s", tag->tagName.c_str());
        }

        if (type == SLexItem::Type::Identifier) {
            SLexIdentifier* id = static_cast<SLexIdentifier*>(item.get());
            // printf("id name is %s\n", id->idName.c_str());
            errorStream.add(" id name is ");
            errorStream.add(id->idName);
        }
        return errorStream.str();
    }

    

    if (outParsedInstrument->headings.empty()) {
        return "no groups or regions";
    }

  //  SQINFO("and here is parser ourput");
  //  outParsedInstrument->_dump();

    return sError;
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

/* dummy version
SParse::Result SParse::matchHeadingGroup(SInstrumentPtr inst, SLexPtr lex) {
    assert(false);
    Result result;
    return result;
}
*/

// I think now this just needs to match a single heading
#if 1
SParse::Result SParse::matchHeadingGroup(SInstrumentPtr inst, SLexPtr lex) {
    bool matchedOneHeading = false;

    SHeadingPtr theHeading;
    Result result = matchSingleHeading(lex, theHeading);
    if (result.res == Result::ok && theHeading) {
        inst->headings.push_back(theHeading);
    }
    return result;

#if 0
    switch (result.res) {
        case Result::ok:
            assert(theHeading);
            inst->headings.push_back(theHeading);
            return result;
            break;
        case Result::error:
            return result;
            break;
        case Result::no_match:
            // if we match no headings, then we don't look for more,
            // but it's not an error. region with no headings is ok
            assert(result.errorMessage.empty());
            result.res = Result::ok;
            return result;
    }
    #endif

#if 0
    // There must be a better way
    int lineNumber = 0;
    if (lex->next()) {
        lineNumber = lex->next()->lineNumber;
    } else
        assert(false);

   // Result result;
    SHeadingPtr newHeading = std::make_shared<SHeading>(lineNumber);
    assert(false);
#endif
#if 0
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
#endif
    assert(false);
    return result;
}
#endif

#if 0
static std::set<std::string> headingTags = {
    {"region"},
    {"group"},
    {"global"},
    {"control"},
    {"master"},
    {"curve"},
    {"effect"},
    {"midi"},
    {"sample"}};
#endif
static std::map<std::string, SHeading::Type> headingTags = {
    {"region", SHeading::Type::Region},
    {"group", SHeading::Type::Group},
    {"global", SHeading::Type::Global},
    {"control", SHeading::Type::Control},
    {"master", SHeading::Type::Master},
    {"curve", SHeading::Type::Curve},
    {"effect", SHeading::Type::Effect},
    {"midi", SHeading::Type::Midi},
    {"sample", SHeading::Type::Sample}
};

#if 0
static bool isHeadingName(const std::string& s) {
    // SQINFO("checking heading name %s", s.c_str());
    return headingTags.find(s) != headingTags.end();
}
#endif
SHeading::Type getHeadingType(const std::string& s) {
    auto it = headingTags.find(s);
    if (it == headingTags.end()) {
        return SHeading::Type::Unknown; 
    }
    return it->second;
}

SParse::Result SParse::matchSingleHeading(SLexPtr lex, SHeadingPtr& outputHeading) {
    Result result;

    auto tok = lex->next();
    if (!tok) {
        result.res = Result::no_match;
        return result; 
    }

    SHeading::Type headingType = getHeadingType(getTagName(tok));
    if (headingType == SHeading::Type::Unknown) {
        result.res = Result::no_match;
        return result; 
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
        return result;
    }

    int lineNumber = 0;
    if (lex->next()) {
        lineNumber = lex->next()->lineNumber;
    }
    outputHeading = std::make_shared<SHeading>(headingType, lineNumber);

    // it it's not a heading we know or care about, just drop the values

    outputHeading->values = std::move(keysAndValues);
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
        result.errorMessage = "= unexpected end of tokens";
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
