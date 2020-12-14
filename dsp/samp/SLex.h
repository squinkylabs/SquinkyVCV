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
    SLexItemPtr next() {
        return currentIndex < int(items.size()) ? items[currentIndex] : nullptr;
    }
    void consume() {
        currentIndex++;
        // printf("after lex::consume, index = %d\n", currentIndex);
    }
    void _dump() const;
    int _index() const {
        return currentIndex;
    }
    void validate() const;
private:
    // return true if no error
    bool procNextChar(char c);
    bool procFreshChar(char c);
    bool procNextTagChar(char c);
    bool procNextCommentChar(char c);
    bool procEnd();
    bool proxNextIdentifierChar(char c);

    bool inComment = false;
    bool inTag = false;
    bool inIdentifier = false;
    std::string curItem;

    int currentIndex = 0;

    
    static void validateName(const std::string& );

   
};

