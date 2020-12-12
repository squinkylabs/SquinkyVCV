

#include "SLex.h"

#include <assert.h>
SLexPtr SLex::go(const std::string& s) 
{
    SLexPtr result = std::make_shared<SLex>();
    for (const char& c : s) {
        bool ret = result->procNextChar(c);
        if (!ret) {
            return nullptr;
        }
    }
    bool ret = result->procEnd();
    return ret ? result : nullptr;;
}

bool SLex::procNextChar(char c) {
    if (!inTag && !inIdentifier) {
        return procFreshChar(c);
    } else if (inTag) {
        return procNextTagChar(c);
    } else {
        assert(inIdentifier);
        return proxNextIdentifierChar(c);
    }
}

bool SLex::procFreshChar(char c) {
    if (c == '<') {
        inTag = true;
        return true;
    }

    if (c == '=') {
        items.push_back(std::make_shared<SLexEqual>());
        return true;
    }

    inIdentifier = true;
    curItem.clear();
    curItem += c;
    return true;
}

bool SLex::procNextTagChar(char c) {
    if (c == '<') {
        printf("nested tag\n");
        return false;
    }
    if (c == '>') {
        items.push_back(std::make_shared<SLexTag>(curItem));
        curItem.clear();
        inTag = false;
        return true;
    }

    curItem += c;           // do we care about line feeds?
    return true;
}

bool SLex::procEnd() {
    if (inIdentifier) {
        items.push_back(std::make_shared<SLexIdentifier>(curItem));
        curItem.clear();
        return true;
    }

    if (inTag) {
        printf("final tag unterminated\n");
        return false;
    }

    return true;
}
bool SLex::proxNextIdentifierChar(char c) {
     if (c == '<' || c == '<' || c == '=') {
        items.push_back(std::make_shared<SLexIdentifier>(curItem));
        curItem.clear();
        return procFreshChar(c);
    }
    curItem += c;
    return true;
}