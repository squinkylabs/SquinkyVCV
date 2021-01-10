

#include "SLex.h"

#include <assert.h>

#include "SqLog.h"
SLexPtr SLex::go(const std::string& s) {
    int count = 0;
    SLexPtr result = std::make_shared<SLex>();

    for (const char& c : s) {
        if (c == '\n') {
            ++result->currentLine;
        }
        bool ret = result->procNextChar(c);
        if (!ret) {
            return nullptr;
        }
        ++count;
    }
    bool ret = result->procEnd();
    return ret ? result : nullptr;
    ;
}

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
        printf("tok[%d] ", i);
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
}

bool SLex::procNextChar(char c) {
    if (!inTag && !inIdentifier && !inComment) {
        return procFreshChar(c);
    } else if (inTag) {
        return procNextTagChar(c);
    } else if (inComment) {
        return procNextCommentChar(c);
    } else {
        assert(inIdentifier);
        return proxNextIdentifierChar(c);
    }
}

bool SLex::procNextCommentChar(char c) {
    if (c == 10 || c == 13) {
        inComment = false;
    }
    return true;
}

bool SLex::procFreshChar(char c) {
    // printf("proc fresh char>%c<\n", c);
    if (isspace(c)) {
        return true;  // eat whitespace
    }
    if (c == '<') {
        inTag = true;
        return true;
    }

    if (c == '=') {
        addCompletedItem(std::make_shared<SLexEqual>(currentLine), false);
        return true;
    }

    if (c == '/') {
        inComment = true;
        return true;
    }

    inIdentifier = true;
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
        inTag = false;
        return true;
    }

    curItem += c;  // do we care about line feeds?
                   // printf("141, curItem = %s\n", curItem.c_str());
    validateName(curItem);
    return true;
}

bool SLex::procEnd() {
    if (inIdentifier) {
        validateName(curItem);
        addCompletedItem(std::make_shared<SLexIdentifier>(curItem, currentLine), true);
        return true;
    }

    if (inTag) {
        //printf("final tag unterminated\n");terminatingSpace
        return false;
    }

    return true;
}

bool SLex::proxNextIdentifierChar(char c) {
    // printf("proc next ident char = >%c<\n", c);
#if 0
    printf("itesm size =%d\n", int(items.size()));
    if (items.size() >= 2) {
        printf("back type = %d\n", items.back()->itemType);
        printf("before that %d\n", items[items.size() - 2]->itemType);
    }
#endif
    if (c == '=') {
        return procEqualsSignInIdentifier();
    }
    // terminate identifier on these, but proc them
    // TODO, should the middle one be '>'? is that just an error?
    if (c == '<' || c == '<' || c == '=') {
        addCompletedItem(std::make_shared<SLexIdentifier>(curItem, currentLine), true);
        inIdentifier = false;
        return procFreshChar(c);
    }

    const bool terminatingSpace = isspace(c) && (lastIdentifier != "sample");
    // terminate on these, but don't proc
    if (terminatingSpace) {
        addCompletedItem(std::make_shared<SLexIdentifier>(curItem, currentLine), true);
        inIdentifier = false;
        return true;
    }
    assert(inIdentifier);
    curItem += c;
    validateName(curItem);
    return true;
}

bool SLex::procEqualsSignInIdentifier() {
    if (lastIdentifier == "sample") {
        // If we get an equals sign in the middle of a sample file name, then we need to adjust.
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
        auto searchIndex = lastSpacePos;
        while (searchIndex >= 0 && curItem.at(searchIndex) == ' ') {
            filenameEndIndex = searchIndex;
            searchIndex--;
        }
        std::string fileName = curItem.substr(0, filenameEndIndex);

        addCompletedItem(std::make_shared<SLexIdentifier>(fileName, currentLine), true);
        addCompletedItem(std::make_shared<SLexIdentifier>(nextId, currentLine), true);
        inIdentifier = false;
        return procFreshChar('=');
    } else {
        // if it's not a sample file, then process normally. Just finish identifier
        // and go on with the equals sign/
        addCompletedItem(std::make_shared<SLexIdentifier>(curItem, currentLine), true);
        inIdentifier = false;
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
        lastIdentifier = ident->idName;
        // printf("just pushed new id : >%s<\n", lastIdentifier.c_str());
    }
}

std::string SLexItem::lineNumberAsString() const {
    char buf[100];
    sprintf_s(buf, "%d", lineNumber);
    return buf;
}