#pragma once
#include <memory>
#include <string>
#include <vector>

class SLex;
using SLexPtr = std::shared_ptr<SLex>;

class SLexItem {
public:
    enum class Type {
        Tag,
        Identifier,
        Equal
    };
    SLexItem(Type t) : itemType(t) {}


    const Type itemType;

};

class SLexTag : public SLexItem {
public:
    SLexTag(const std::string sName) : SLexItem(Type::Tag), tagName(sName) {}
    const std::string tagName;
};

class SLexEqual : public SLexItem {
public:
    SLexEqual() : SLexItem(Type::Equal) {}
};

class SLexIdentifier : public SLexItem {
public:
    SLexIdentifier(const std::string sName) : SLexItem(Type::Identifier), idName(sName) {}
    const std::string idName;
};


using SLexItemPtr = std::shared_ptr<SLexItem>;

class SLex
{
public: 
    static SLexPtr go(const std::string& s);
    std::vector<SLexItemPtr> items;
private:
    // return true if no error
    bool procNextChar(char c);
    bool procFreshChar(char c);
    bool procNextTagChar(char c);
    bool procEnd();
    bool proxNextIdentifierChar(char c);


    bool inTag = false;
    bool inIdentifier = false;
    std::string curItem;

   
};

