#include "LexContext.h"

#include <assert.h>

#include <fstream>

#include "SqLog.h"
#include "SqStream.h"

LexContext::LexContext(const std::string& initialContent) : currentContent(initialContent) {
    includeRecursionDepth = 1;
    scopes.push_back(std::make_shared<LexFileScope>());
}

#if 0
LexContext::LexContext(const FilePath& initialFile) {
    assert(false);
}
#endif

void LexContext::addDefine(const std::string& defineVarName, const std::string& defineVal) {
    // SQINFO("addDefine var=%s val=%s", defineVarName.c_str(), defineVal.c_str());
    if (defineVarName.empty()) {
        SQWARN("trying to add empty define");
        return;
    }
    if (defineVarName.front() != '$') {
        SQWARN("var does not start with $: %s", defineVarName.c_str());
        return;
    }
    assert(!scopes.empty());
    scopes.back()->addDefine(defineVarName, defineVal);
}

void LexContext::applyDefine(std::string* theString) {
    // SQINFO("applyDefine: %s", theString->c_str());
    
    // search the scopes from closest to farthest,
    // looking for a scope that has this string.
    for (auto it = scopes.rbegin(); it !=scopes.rend(); ++it) {
        LexFileScopePtr scope = *it;
        bool found = scope->applyDefine(theString);
        if (found) {
            return;
        }
    }
}

bool LexContext::popOneLevel() {
    --includeRecursionDepth;
    assert(scopes.size() > 0);
    scopes.pop_back();
    return true;
}

bool LexContext::pushOneLevel(const std::string& relativePath, int currentLine) {
    ++includeRecursionDepth;
    if (includeRecursionDepth > 10) {
        errorString_ = "Include nesting too deep";
        return false;
    }
    if (relativePath.front() != '"' || relativePath.back() != '"') {
        errorString_ = "Include filename not quoted";
        return false;
    }

    std::string rawFilename = relativePath.substr(1, relativePath.length() - 2);
    if (rootFilePath.empty()) {
        errorString_ = "Can't resolve include with no root path";
        return false;
    }

    scopes.push_back(std::make_shared<LexFileScope>());
    FilePath origPath(rootFilePath);
    FilePath origFolder = origPath.getPathPart();
    FilePath namePart(rawFilename);
    FilePath fullPath = origFolder;
    fullPath.concat(namePart);
    // SQINFO("make full include path: %s", fullPath.toString().c_str());

    std::string sIncludeContent;

    // if a unit test is providing content, use it.
    // otherwise open the include file.
    auto it = testFolders.find(fullPath.toString());
    if (it != testFolders.end()) {
        sIncludeContent = it->second;
    } else {
        std::ifstream t(fullPath.toString());
        if (!t.good()) {
            //  printf("can't open file\n");
            // return "can't open source file: " + sPath;
            SQWARN("can't open include %s", fullPath.toString().c_str());
            SQWARN("root = %s", rootFilePath.toString().c_str());

            SqStream s;
            s.add("Can't open ");
            s.add(rawFilename);
            s.add(" included");
            s.add(" at line ");
            s.add(currentLine + 1);
            errorString_ = s.str();
            return false;
        }
        std::string str((std::istreambuf_iterator<char>(t)),
                        std::istreambuf_iterator<char>());
        sIncludeContent = std::move(str);
    }
    if (sIncludeContent.empty()) {
        errorString_ = ("Include file empty ");
        return false;
    }
    currentContent = std::move(sIncludeContent);
    return true;
}

void LexContext::addTestFolder(const FilePath& folder, const std::string& content) {
    SQINFO("adding a mapping for folder >%s<", folder.toString().c_str());
    testFolders[folder.toString()] = content;
}

//////////////////////////////////////////////////////////////////////////////

bool LexFileScope::applyDefine(std::string* theString) {
    // SQINFO("apply define %s", theString->c_str());
    if (theString->empty()) {
        return false;
    }
    for (auto it : defines) {
        auto pos = theString->find(it.first);
        if (pos != std::string::npos) {
            const size_t len = it.first.size();

            // SQINFO("found a match, val = %s", it.second.c_str());
            theString->replace(pos, len, it.second);
            return true;
        }
    }
    return false;
}
