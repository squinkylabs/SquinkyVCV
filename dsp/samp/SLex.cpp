

#include "SLex.h"
#include "LexContext.h"

#include <assert.h>

#include <fstream>

#include "FilePath.h"
#include "SqLog.h"
#include "SqStream.h"

//SLex::SLex(std::string* errorText, int includeDepth, const FilePath* filePath) : outErrorStringPtr(errorText), includeRecursionDepth(includeDepth), rootFilePath(filePath) {
//}

SLex::SLex(LexContextPtr ctx) : context(ctx) {
}
#if 0
SLexPtr SLex::go(const std::string& sContent, std::string* errorText, int includeDepth, const FilePath* yourFilePath) {
    // song and dance to work around MS bug.
    SLex* lx = new SLex(errorText, includeDepth, yourFilePath);
    return goCommon(lx, sContent);
}
#endif

 SLexPtr SLex::go(const std::string& sContent) {
     LexContextPtr ctx = std::make_shared<LexContext>(sContent);
     return go(ctx);
 }
 
 SLexPtr SLex::go(LexContextPtr context) {
  
     SLex* lx = new SLex(context);
     return goCommon(lx, context);
 }

#if 0
 static SLexPtr goCommon(SLex* lx, LexContextPtr ctx) {
     assert(false);
     SLexPtr result(lx);
     return result;
 }
#endif

 SLexPtr SLex::goRecurse(LexContextPtr context) {
     SLex* lx = new SLex(context);
  //   SQINFO("will recurse into content: %s", context->getCurrentContent().c_str());
 //    assert( context->includeRecursionDepth > 0);
  //   SQINFO("depth = %d", context->includeRecursionDepth);
     return goCommon(lx, context);
 }

#if 0
SLexPtr SLex::goRecurse(const FilePath* rootFilePath, const std::string& sContent, std::string* errorText, int includeDepth, const std::string& dbg) {
    int count = 0;
    // song and dance to work around MS bug.
    SLex* lx = new SLex(errorText, includeDepth, rootFilePath);
    return goCommon(lx, sContent);
}
#endif


SLexPtr SLex::goCommon(SLex* lx, LexContextPtr ctx) {
    int count = 0;
    SLexPtr result(lx);

    std::string sContent = ctx->getCurrentContent();
    for (size_t i = 0; i < sContent.size(); ++i) {
        const char c = sContent[i];
        const char nextC = (i >= (sContent.size() - 1)) ? -1 : sContent[i + 1];
        if (c == '\n') {
            ++result->currentLine;
        }

        bool ret = result->procNextChar(c, nextC);
        if (!ret) {
            return nullptr;
        }
        ++count;
    }
    //SQINFO("in goCommon will dec recusion is %d", ctx->includeRecursionDepth);
   // ctx->includeRecursionDepth--;
    ctx->popOneLevel();
    bool ret = result->procEnd();
    return ret ? result : nullptr;
}
#if 0   // old one
SLexPtr SLex::goCommon(SLex* lx, const std::string& sContent) {
    int count = 0;
    SLexPtr result(lx);

    for (size_t i=0; i< sContent.size(); ++i) {
        const char c = sContent[i];
        const char nextC = (i >= (sContent.size() - 1)) ? -1 : sContent[i + 1];
        if (c == '\n') {
            ++result->currentLine;
        }
      
        bool ret = result->procNextChar(c, nextC);
        if (!ret) {
            return nullptr;
        }
        ++count;
    }
    bool ret = result->procEnd();
    return ret ? result : nullptr;
}
#endif


void SLex::validateName(const std::string& name) {
// TODO: now that file names can have spaces, we can't do this.
// maybe we should check in the parser or compiler, where we know what's what?
#if 0
    for (char const& c : name) {
        assert(!isspace(c));
    }
#endif
}

void SLex::validate() const {
    for (auto item : items) {
        switch (item->itemType) {
            case SLexItem::Type::Tag: {
                SLexTag* tag = static_cast<SLexTag*>(item.get());
                validateName(tag->tagName);
            } break;
            case SLexItem::Type::Identifier: {
                SLexIdentifier* id = static_cast<SLexIdentifier*>(item.get());
                validateName(id->idName);
            } break;
            case SLexItem::Type::Equal:
                break;
            default:
                assert(false);
        }
    }
}

void SLex::_dump() const {
    printf("dump lexer, there are %d tokens\n", (int)items.size());
    for (int i = 0; i < int(items.size()); ++i) {
        // for (auto item : items) {
        auto item = items[i];
        printf("tok[%d] #%d ", i, item->lineNumber);
        switch (item->itemType) {
            case SLexItem::Type::Tag: {
                SLexTag* tag = static_cast<SLexTag*>(item.get());
                printf("tag=%s\n", tag->tagName.c_str());
            } break;
            case SLexItem::Type::Identifier: {
                SLexIdentifier* id = static_cast<SLexIdentifier*>(item.get());
                printf("id=%s\n", id->idName.c_str());
            } break;
            case SLexItem::Type::Equal:
                printf("Equal\n");
                break;
            default:
                assert(false);
        }
    }
    fflush(stdout);
}

bool SLex::procNextChar(char c, char nextC) {
    //SQINFO("proc next: %c", c);
    switch (state) {
        case State::Ready:
            return procFreshChar(c, nextC);
        case State::InTag:
            return procNextTagChar(c);
        case State::InComment:
            return procNextCommentChar(c);
        case State::InInclude:
            return procNextIncludeChar(c);
        case State::InIdentifier:
            return procNextIdentifierChar(c);
        case State::InDefine:
            return procStateNextDefineChar(c);
        case State::InHash:
            return procStateNextHashChar(c);
        default:
            assert(false);
    }
    assert(false);
    return true;
}



bool SLex::error(const std::string& err) {
    SqStream st;
    st.add(err);
    st.add(" at line ");
    st.add(currentLine + 1);
    //*outErrorStringPtr = st.str();
    context->logError(st.str());
    return false;
}

#if 0
bool SLex::error(const std::string& err) {

    if (outErrorStringPtr) {
        SqStream st;
        st.add(err);
        st.add(" at line ");
        st.add(currentLine + 1);
        *outErrorStringPtr = st.str();
    }

    return false;
}
#endif

bool SLex::procStateNextHashChar(char c) {
    switch (c) {
        case 'i':
            state = State::InInclude;
            includeSubState = IncludeSubState::MatchingOpcode;
            curItem = "i";
            // SQINFO("going into incl, curItem=%s", curItem.c_str());
            return true;
        case 'd':
            state = State::InDefine;
            defineSubState = DefineSubState::MatchingOpcode;
            curItem = "d";
            // SQINFO("going into define, curItem=%s", curItem.c_str());
            return true;
        default:
            // TODO: tests case #xx
            assert(false);
            return false;
    }
}

bool SLex::procStateNextDefineChar(char c) {
    static std::string defineStr("define");
    switch (defineSubState) {
        case DefineSubState::MatchingOpcode:
            curItem += c;
            if (defineStr.find(curItem) != 0) {
                SQINFO("bad item: >%s<", curItem.c_str());
                return error("Malformed #define");
            }
            if (curItem == defineStr) {
                defineSubState = DefineSubState::MatchingSpace;
                spaceCount = 0;
            }
            return true;

        case DefineSubState::MatchingSpace:
            if (isspace(c)) {
                spaceCount++;
                return true;
            }
            if (spaceCount > 0) {
                defineSubState = DefineSubState::MatchingLhs;
                return true;
            }

            assert(false);
            return false;

        case DefineSubState::MatchingSpace2:
            if (isspace(c)) {
                spaceCount++;
                return true;
            }
            if (spaceCount > 0) {
                defineSubState = DefineSubState::MatchingRhs;
                // need to save off char we just saw (in the future, if we care about the content
                return true;
            }

            assert(false);
            return false;

        case DefineSubState::MatchingLhs:
            // SQINFO("match lhs, got %c", c);
            if (isspace(c)) {
                defineSubState = DefineSubState::MatchingSpace2;
                spaceCount = 1;
                return true;
            }
            return true;
        case DefineSubState::MatchingRhs:
            //SQINFO("match rhs, got %c", c);
            //SQINFO("Line: %d", currentLine);
            if (isspace(c)) {
                //SQINFO("in match, is space");
                // when we finish rhs, we are done
                curItem.clear();
                // 3 continue lexing
                state = State::Ready;
                return true;
            }
            return true;

        default:
            assert(false);
    }
    return true;
}

bool SLex::procNextIncludeChar(char c) {
    static std::string includeStr("include");
    switch (includeSubState) {
        case IncludeSubState::MatchingOpcode:
            curItem += c;
            if (includeStr.find(curItem) != 0) {
                SQINFO("bad item: >%s<", curItem.c_str());
                return error("Malformed #include");
            }
            if (curItem == includeStr) {
                includeSubState = IncludeSubState::MatchingSpace;
                spaceCount = 0;
            }
            return true;

        case IncludeSubState::MatchingSpace:
            if (isspace(c)) {
                spaceCount++;
                return true;
            }
            if (spaceCount > 0) {
                includeSubState = IncludeSubState::MatchingFileName;
                curItem = c;
                assert(curItem.size() == 1);
                return true;
            }
            assert(false);
            return false;

        case IncludeSubState::MatchingFileName:
            if (c == '\n') {
                assert(false);
                return false;
            }
            curItem += c;
            if ((c == '"') && curItem.size() > 1) {
                // OK, here we found a file name!
                return handleIncludeFile(curItem);
            }
            return true;

        default:
            assert(false);
    }

    // for now just keep eating chars
    //assert(false);
    return true;
}

bool SLex::procNextCommentChar(char c) {
    if (c == 10 || c == 13) {
        //inComment = false;
        state = State::Ready;
    }
    return true;
}

bool SLex::procFreshChar(char c, char nextC) {
    if (isspace(c)) {
        return true;  // eat whitespace
    }
    switch (c) {
        case '<':
            state = State::InTag;
            return true;
        case '/':
            if (nextC == '/') {
                state = State::InComment;
                return true;
            }
            break;          
        case '=':
            addCompletedItem(std::make_shared<SLexEqual>(currentLine), false);
            return true;
        case '#':
            state = State::InHash;
            return true;
    }

    // inIdentifier = true;
    state = State::InIdentifier;
    curItem.clear();
    curItem += c;
    //printf("119, curItem = %s\n", curItem.c_str());
    validateName(curItem);
    return true;
}

bool SLex::procNextTagChar(char c) {
    // printf("nextteag=%c\n", c);
    if (isspace(c)) {
        return false;  // can't have white space in the middle of a tag
    }
    if (c == '<') {
        // printf("nested tag\n");
        return false;
    }
    if (c == '>') {
        validateName(curItem);
        addCompletedItem(std::make_shared<SLexTag>(curItem, currentLine), true);
        //inTag = false;
        state = State::Ready;
        return true;
    }

    curItem += c;  // do we care about line feeds?
                   // printf("141, curItem = %s\n", curItem.c_str());
    validateName(curItem);
    return true;
}

bool SLex::procEnd() {
    if (state == State::InIdentifier) {
        validateName(curItem);
        addCompletedItem(std::make_shared<SLexIdentifier>(curItem, currentLine), true);
        return true;
    }

    if (state == State::InTag) {
        //printf("final tag unterminated\n");terminatingSpace
        return false;
    }

    return true;
}

bool SLex::procNextIdentifierChar(char c) {
    if (c == '=') {
        return procEqualsSignInIdentifier();
    }

    // check for a comment terminating a string
    if (c == '/') {
        if (lastCharWasForwardSlash) {
            if (!curItem.empty()) {
                assert(curItem.back() == '/');
                curItem.pop_back();
            }
            // remove trailing space
            while (!curItem.empty() && isspace(curItem.back())) {
                curItem.pop_back();
            }

            addCompletedItem(std::make_shared<SLexIdentifier>(curItem, currentLine), true);
            state = State::InComment;
            return true;
        }
        lastCharWasForwardSlash = true;
    } else {
        lastCharWasForwardSlash = false;
    }
    // terminate identifier on these, but proc them
    // TODO, should the middle one be '>'? is that just an error?
    if (c == '<' || c == '<' || c == '=' || c == '\n') {
        addCompletedItem(std::make_shared<SLexIdentifier>(curItem, currentLine), true);
        //inIdentifier = false;
        state = State::Ready;
        return procFreshChar(c);
    }

    // We only terminate on a space if we are not parsing a String type opcode
    // const bool terminatingSpace = isspace(c) && (lastIdentifierType != SamplerSchema::OpcodeType::String);
    const bool terminatingSpace = isspace(c) && !lastIdentifierIsString;
    // terminate on these, but don't proc
    if (terminatingSpace) {
        addCompletedItem(std::make_shared<SLexIdentifier>(curItem, currentLine), true);
        //inIdentifier = false;
        state = State::Ready;
        return true;
    }
    //assert(inIdentifier);
    assert(state == State::InIdentifier);
    curItem += c;
    validateName(curItem);
    return true;
}

bool SLex::procEqualsSignInIdentifier() {
    if (lastIdentifierIsString) {
        // If we get an equals sign in the middle of a sample file name (or other string), then we need to adjust.
        // for things other than sample we don't accept spaces, so there is no issue.

        // The last space is going to the the character right before the next identifier.
        auto lastSpacePos = curItem.rfind(' ');
        if (lastSpacePos == std::string::npos) {
            SQWARN("equals sign found in identifier at line %d", currentLine);
            return false;  // error
        }
        // todo: multiple spaces
        // std::string fileName = curItem.substr(0, lastSpacePos);

        std::string nextId = curItem.substr(lastSpacePos + 1);
        auto filenameEndIndex = lastSpacePos;
        int searchIndex = int(lastSpacePos);
        while (searchIndex >= 0 && curItem.at(searchIndex) == ' ') {
            filenameEndIndex = searchIndex;
            searchIndex--;
        }
        std::string fileName = curItem.substr(0, filenameEndIndex);

        addCompletedItem(std::make_shared<SLexIdentifier>(fileName, currentLine), true);
        addCompletedItem(std::make_shared<SLexIdentifier>(nextId, currentLine), true);
        //inIdentifier = false;
        state = State::Ready;
        return procFreshChar('=');
    } else {
        // if it's not a sample file, then process normally. Just finish identifier
        // and go on with the equals sign/
        addCompletedItem(std::make_shared<SLexIdentifier>(curItem, currentLine), true);
        // inIdentifier = false;
        state = State::Ready;
        return procFreshChar('=');
    }
}

void SLex::addCompletedItem(SLexItemPtr item, bool clearCurItem) {
    items.push_back(item);
    if (clearCurItem) {
        curItem.clear();
    }
    if (item->itemType == SLexItem::Type::Identifier) {
        SLexIdentifier* ident = static_cast<SLexIdentifier*>(item.get());
        lastIdentifierIsString = SamplerSchema::isFreeTextType(ident->idName);
        // printf("just pushed new id : >%s<\n", lastIdentifier.c_str());
    }
}

std::string SLexItem::lineNumberAsString() const {
    char buf[100];
    // sprintf_s(buf, "%d", lineNumber);
    snprintf(buf, sizeof(buf), "%d", lineNumber);
    return buf;
}

bool SLex::handleIncludeFile(const std::string& relativeFileName) {
   //  SQINFO("SLex::handleIncludeFile %s", fileName.c_str());
    assert(!relativeFileName.empty());
  

 //   if (context->includeRecursionDepth > 10) {
  //      return error("include nesting too deep");
 //   }

  //  const auto before = context->includeRecursionDepth;

    //  bool pushOneLevel(const std::string& relativePath, std::string* fileContents);
    bool bOK = context->pushOneLevel(relativeFileName, currentLine);
    if (!bOK) {
        return false;
    }

    // now need to recurse
    auto includeLexer = SLex::goRecurse(context);
    if (!includeLexer) {
        return false;  // error should already be in outErrorStringPtr
    }
    //assert(context->includeRecursionDepth == before + 1);
    // 2) copy the tokens from include to this.
    this->items.insert(
        this->items.end(),
        std::make_move_iterator(includeLexer->items.begin()),
        std::make_move_iterator(includeLexer->items.end()));
    // SQINFO("finished incl, curItem=%s", curItem.c_str());
    curItem.clear();
    // 3 continue lexing
    state = State::Ready;
    // SQINFO("back frm %s", fullPath.toString().c_str());

    return true;
}

#if 0
bool SLex::handleIncludeFile(const std::string& fileName) {
   //  SQINFO("SLex::handleIncludeFile %s", fileName.c_str());
    assert(!fileName.empty());
    assert(false);

    if (includeRecursionDepth > 10) {
        return error("include nesting too deep");
    }

// part 2 - open new file and get contents
    if (fileName.front() != '"' || fileName.back() != '"') {
        return error("Include filename not quoted");
    }
    std::string rawFilename = fileName.substr(1, fileName.length() - 2);
    if (!rootFilePath) {
        return error("Can't resolve include with no context");
    }
    FilePath origPath(*rootFilePath);
    FilePath origFolder = origPath.getPathPart();
    FilePath namePart(rawFilename);
    FilePath fullPath = origFolder;
    fullPath.concat(namePart);
    //SQINFO("make full include path: %s", fullPath.toString().c_str());

    std::ifstream t(fullPath.toString());
    if (!t.good()) {
        //  printf("can't open file\n");
        // return "can't open source file: " + sPath;
        SQWARN("can't open include");

        SqStream s;
        s.add("Can't open ");
        s.add(rawFilename);
        s.add(" included");
        return error(s.str());
    }
    std::string str((std::istreambuf_iterator<char>(t)),
                    std::istreambuf_iterator<char>());
    if (str.empty()) {
        return error("Include file empty ");
    }
    //SQINFO("going into %s", fullPath.toString().c_str());

    // ok, we have the content of the include.
    // we must:
    // 1) lex it.
    //    static SLexPtr go(const std::string& sContent, std::string* errorText = nullptr, int includeDepth = 0, const FilePath* yourFilePath = nullptr);

// part 3, using new file to recurse
    auto includeLexer = SLex::goRecurse(rootFilePath, str, outErrorStringPtr, includeRecursionDepth + 1, namePart.toString());
    if (!includeLexer) {
        return false;  // error should already be in outErrorStringPtr
    }
    // 2) copy the tokens from include to this.
    this->items.insert(
        this->items.end(),
        std::make_move_iterator(includeLexer->items.begin()),
        std::make_move_iterator(includeLexer->items.end()));
    // SQINFO("finished incl, curItem=%s", curItem.c_str());
    curItem.clear();
    // 3 continue lexing
    state = State::Ready;
    // SQINFO("back frm %s", fullPath.toString().c_str());

    return true;
}
#endif