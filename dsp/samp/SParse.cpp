
#include "SParse.h"

#include "FilePath.h"
#include "SInstrument.h"
#include "SLex.h"
#include "SqLog.h"
#include "SqStream.h"
#include "share/windows_unicode_filenames.h"
#include <assert.h>

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
    //lex->_dump();

    assert(false);  // finish me
    std::string sError = "finish me 97";
  //  std::string sError = matchHeadingGroups(outParsedInstrument, lex);
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

    assert(false);
    sError = "finish me 129";
#if 0
    if (outParsedInstrument->groups.empty()) {
        return "no groups or regions";
    }
#endif
    return sError;
}


