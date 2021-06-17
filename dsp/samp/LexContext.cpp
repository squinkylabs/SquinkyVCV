#include "LexContext.h"

#include "SqLog.h"
#include "SqStream.h"

#include <assert.h>
#include <fstream>


LexContext::LexContext(const std::string& initialContent) : currentContent(initialContent) {
}

LexContext::LexContext(const FilePath& initialFile) {
    assert(false);
}

bool LexContext::pushOneLevel(const std::string& relativePath, int currentLine) {
    
    ++includeRecursionDepth;
    if (relativePath.front() != '"' || relativePath.back() != '"') {
        errorString_ = "Include filename not quoted";
        return false;
    }

    std::string rawFilename = relativePath.substr(1, relativePath.length() - 2);
    if (rootFilePath.empty()) {
        errorString_ = "Can't resolve include with no root path";
        return false;
    }
    FilePath origPath(rootFilePath);
    FilePath origFolder = origPath.getPathPart();
    FilePath namePart(rawFilename);
    FilePath fullPath = origFolder;
    fullPath.concat(namePart);
    //SQINFO("make full include path: %s", fullPath.toString().c_str());

    std::ifstream t(fullPath.toString());
    if (!t.good()) {
        //  printf("can't open file\n");
        // return "can't open source file: " + sPath;
        SQWARN("can't open include");

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
    if (str.empty()) {
        errorString_ = ("Include file empty ");
        return false;
    }
    currentContent = std::move(str);
    return true;
}