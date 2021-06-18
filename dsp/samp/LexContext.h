#pragma once

#include "FilePath.h"
#include <assert.h>
#include <list>
#include <map>
#include <string>
#include <memory>

class LexFileScope {
public:
    void addDefine(const std::string& defineVarName, const std::string& defineVal) {
        defines[defineVarName] = defineVal;
    }
    bool applyDefine(std::string*);
private:
    std::map<std::string, std::string> defines;
};
using LexFileScopePtr = std::shared_ptr<LexFileScope>;

/**
 * LexContext it a concrete class we use
 * to pass file information to lexer and to keep #define scopes
 * 
 * Some methods are virtual just so the test version can overload them
 */
class LexContext {
public:
   // static SLexPtr go(const std::string& sContent, std::string* errorText = nullptr, int includeDepth = 0, const FilePath* yourFilePath = nullptr);
    LexContext(const std::string& initialContent);
  //  LexContext(const FilePath& initialFile);

    void addRootPath(const FilePath& fp) {
        assert(rootFilePath.empty());
        rootFilePath = fp;
    }

    /**
     * opens a new file (presumably from an include).
     * after this "current contents" will be from the new file
     * if ret == false, will log error internally.
     * 
     * @param sourceLine used for formatting error message
     */
    bool pushOneLevel(const std::string& relativePath, int sourceLine);
    bool popOneLevel();

    void addDefine(const std::string& defineVarName, const std::string& defineVal);

    /**
     * Look up string and replace defined part
     */
    void applyDefine(std::string*);

    std::string getCurrentContent() const { 
        assert(!currentContent.empty());
        return currentContent;
    }

    void logError(const std::string& s) { errorString_ = s; }
    std::string errorString() const { return errorString_; }
    FilePath getRootFilePath() const { return rootFilePath; }

  
private:
    std::string currentContent;
    std::string errorString_;
    FilePath rootFilePath;

    int includeRecursionDepth = 0;
    std::list<LexFileScopePtr> scopes;
};

class TestLexContext : public LexContext {

};

using LexContextPtr = std::shared_ptr<LexContext>;