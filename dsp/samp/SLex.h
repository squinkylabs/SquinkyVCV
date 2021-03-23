#pragma once

#include "SamplerSchema.h"
#include "SqLog.h"

#include <memory>
#include <string>
#include <vector>

class FilePath;
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
    /**
     * @param sContent is the input data to analyze (typically the contents of an SFZ file).
     * @param errorTest is in out parameter for returning lexing errors.
     * @param includeDepth is passed when lexing recursively for #include resolution.
     * @returns lexer full of tokens, or null if error
     */
  
    static SLexPtr go(const std::string& sContent, std::string* errorText = nullptr, int includeDepth = 0, const FilePath* yourFilePath = nullptr);
    SLex(std::string* errorText, int includeDepth, const FilePath* yourFilePath);
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
    bool procNextIncludeChar(char c);
    bool procEnd();
    bool procNextIdentifierChar(char c);
    bool procEqualsSignInIdentifier();
    bool procStateNextDefineChar(char c);
    bool procStateNextHashChar(char c);



    bool error(const std::string&);
    bool handleIncludeFile(const std::string&);

    void addCompletedItem(SLexItemPtr, bool clearCurItem);
    bool handleInclude(const std::string&);

    enum class State {
        Ready,
        InComment,
        InTag,
        InIdentifier,
        InHash,
        InInclude,
        InDefine
    };

    State state = State::Ready;

    enum class IncludeSubState {
        MatchingOpcode,
        MatchingSpace,
        MatchingFileName
    };

    // #define a b
    // a is lhs, b is rhs
    // match: opcode, space, lhs, space2, rhs
    enum class DefineSubState {
        MatchingOpcode,
        MatchingSpace,
        MatchingLhs,
        MatchingSpace2,
        MatchingRhs,
    };

    IncludeSubState includeSubState = IncludeSubState::MatchingOpcode;
    DefineSubState defineSubState = DefineSubState::MatchingOpcode;

    int spaceCount = 0;
    
    std::string curItem;
    bool lastIdentifierIsString = false;
    std::string* const outErrorStringPtr;
    const FilePath* const myFilePath;
    const int includeRecursionDepth;

    int currentIndex = 0;

    // internally it's zero based, but we make it one based for things we expose.
    int currentLine = 0;


    static void validateName(const std::string&);
};
