

#include "SLex.h"

#include <assert.h>
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
    for (char const& c : name) {
        assert(!isspace(c));
    }
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
    if (isspace(c)) {
        return true;  // eat whitespace
    }
    if (c == '<') {
        inTag = true;
        return true;
    }

    if (c == '=') {
        items.push_back(std::make_shared<SLexEqual>(currentLine));
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
        items.push_back(std::make_shared<SLexTag>(curItem, currentLine));
        curItem.clear();
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
        items.push_back(std::make_shared<SLexIdentifier>(curItem, currentLine));
        curItem.clear();
        return true;
    }

    if (inTag) {
        //printf("final tag unterminated\n");
        return false;
    }

    return true;
}
bool SLex::proxNextIdentifierChar(char c) {
    // terminate identifier on these, but proc them
    if (c == '<' || c == '<' || c == '=') {
        items.push_back(std::make_shared<SLexIdentifier>(curItem, currentLine));
        curItem.clear();
        inIdentifier = false;
        return procFreshChar(c);
    }

    // terminate on these, but don't proc
    if (isspace(c)) {
        items.push_back(std::make_shared<SLexIdentifier>(curItem, currentLine));
        curItem.clear();
        inIdentifier = false;
        return true;
    }
    assert(inIdentifier);
    curItem += c;
    //printf("175, curItem = %s\n", curItem.c_str());
    validateName(curItem);
    return true;
}

  std::string SLexItem::lineNumberAsString() const {
       char buf[100];
       sprintf_s(buf, "%d", lineNumber);
       return buf;
  }