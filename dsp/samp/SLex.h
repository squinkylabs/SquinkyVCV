#pragma once

#include <memory>
#include <string>
#include <vector>

#include "SamplerSchema.h"
#include "SqLog.h"

class FilePath;
class SLex;
class LexContext;
using SLexPtr = std::shared_ptr<SLex>;
using LexContextPtr = std::shared_ptr<LexContext>;

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

inline std::string removeTrailingSpace(const std::string& s) {
    const std::string whitespace = " \t";
    const auto strEnd = s.find_last_not_of(whitespace);
    return s.substr(0, strEnd + 1);
}

class SLexIdentifier : public SLexItem {
public:
    SLexIdentifier(const SLexIdentifier&) = delete;

    /**
     * Trailing space it removed from the name.
     * This was done for a bug where spaces after a file name were getting added to the extension,
     * but it's good in general. No ident has trailing spaces.
     */
    SLexIdentifier(const std::string sName, int line) : SLexItem(Type::Identifier, line), idName(removeTrailingSpace(sName)) {
        if (!idName.empty()) {
            assert(idName.back() != ' ');
            assert(idName.back() != '\t');
        }
    }
    std::string idName;
};

using SLexItemPtr = std::shared_ptr<SLexItem>;

class SLex {
public:
    /**
     * This factory should only be called to make the top level Lexer. It should not be used
     * internally to recurse into a new file.
     * 
     * @param sContent is the input data to analyze (typically the contents of an SFZ file).
     * @param errorTest is in out parameter for returning lexing errors.
     * @param includeDepth is passed when lexing recursively for #include resolution.
     * @param yourFilePath is the full path to the file that sContentCame from.
     * 
     * These optional params should be passed for "real" code, but they are not required for a lot of unit test code.
     * 
     * @returns lexer full of tokens, or null if error
     */

    static SLexPtr go(LexContextPtr context);        // full featured factory
    static SLexPtr go(const std::string& sContent);  // limited one, just for test
                                                     // static SLexPtr go(const std::string& sContent, std::string* errorText = nullptr, int includeDepth = 0, const FilePath* yourFilePath = nullptr);

    /**
     * This is the factory for "child" lexers while processing includes
     */
  //  static SLexPtr goRecurse(const FilePath* rootFilePath, const std::string& sContent, std::string* errorText, int includeDepth, const std::string& dbgString);
    static SLexPtr goRecurse(LexContextPtr ctx);

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

    SLex(const SLex&) = delete;
    const SLex& operator=(const SLex&) = delete;

private:
    SLex(std::string* errorText, int includeDepth, const FilePath* rootFilePath);       // old legacy one
    SLex(LexContextPtr);                    // new spiffy one
    const std::string debugString;

    // return true if no error
    // the functions that take a second argument expect it to either be
    // the next character, or -1 for "none" or "not known"
    bool procNextChar(char c, char nextC = -1);
    bool procFreshChar(char c, char nextC = -1);
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
      LexContextPtr const context;

    std::string curItem;
    bool lastIdentifierIsString = false;
    bool lastCharWasForwardSlash = false;

    std::string defineVarName;
    std::string defineValue;

    int currentIndex = 0;

    // internally it's zero based, but we make it one based for things we expose.
    int currentLine = 0;

    static void validateName(const std::string&);
    static SLexPtr goCommon(SLex* lx, LexContextPtr);
};
