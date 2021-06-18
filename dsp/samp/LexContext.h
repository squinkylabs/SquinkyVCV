#pragma once

#include "FilePath.h"
#include <assert.h>
#include <string>
#include <memory>

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
    LexContext(const FilePath& initialFile);

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
      // TODO: hide this
    int includeRecursionDepth = 0;
};

class TestLexContext : public LexContext {

};

using LexContextPtr = std::shared_ptr<LexContext>;