#pragma once

#include "SamplerSchema.h"
#include "SqLog.h"

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
    SLexItem(Type t, int line) : itemType(t), lineNumber(line) {}
    const Type itemType;
    const int lineNumber;
    std::string lineNumberAsString() const;
};

class SLexTag : public SLexItem {
public:
    SLexTag(const std::string sName, int line) : SLexItem(Type::Tag, line), tagName(sName) {}
    const std::string tagName;
};

class SLexEqual : public SLexItem {
public:
    SLexEqual(int line) : SLexItem(Type::Equal, line) {}
};

class SLexIdentifier : public SLexItem {
public:
    SLexIdentifier(const SLexIdentifier&) = delete;
    SLexIdentifier(const std::string sName, int line) : SLexItem(Type::Identifier, line), idName(sName) {}
    const std::string idName;
};

using SLexItemPtr = std::shared_ptr<SLexItem>;

class SLex {
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
    bool procEqualsSignInIdentifier();

    void addCompletedItem(SLexItemPtr, bool clearCurItem);

    bool inComment = false;
    bool inTag = false;
    bool inIdentifier = false;
    std::string curItem;
  //  std::string lastIdentifier;
    SamplerSchema::OpcodeType lastIdentifierType;

    int currentIndex = 0;
    int currentLine = 0;

    static void validateName(const std::string&);
};
