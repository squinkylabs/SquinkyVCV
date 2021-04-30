
#include "FilePath.h"
#include "SamplerSchema.h"
#include "SqLog.h"
#include "asserts.h"
#include "samplerTests.h"

static void testFilePath0() {
    FilePath f("abc");
    assertEQ(f.toString(), "abc");

    assertNE(FilePath::nativeSeparator(), FilePath::foreignSeparator());
}

static void testFilePathFixup() {
    std::string input("\\\\////\\\\\\");
    FilePath f(input);
    const std::string s = f.toString();
    assertNE(s, input);

    bool b = s.find(FilePath::foreignSeparator()) != std::string::npos;
    assert(!b);
}

static void testFilePathFixup2() {
    const char* input = "\\\\////\\\\\\";
    FilePath f(input);
    const std::string s = f.toString();
    assertNE(s, input);

    bool b = s.find(FilePath::foreignSeparator()) != std::string::npos;
    assert(!b);
}

static void testFilePathConcat1() {
    FilePath a("a");
    FilePath b("b");
    a.concat(b);

    std::string s = a.toString();
    assertEQ(s.size(), 3);
    assertEQ(s.at(0), 'a');
    assertEQ(s.at(1), FilePath::nativeSeparator());
    assertEQ(s.at(2), 'b');

    assert(!a.isAbsolute());
}

static void testFilePathConcat2() {
    FilePath a("a/");
    FilePath b("b");
    a.concat(b);

    std::string s = a.toString();
    assertEQ(s.size(), 3);
    assertEQ(s.at(0), 'a');
    assertEQ(s.at(1), FilePath::nativeSeparator());
    assertEQ(s.at(2), 'b');

    assert(!a.isAbsolute());
}

static void testFilePathConcat3() {
    FilePath a("a\\");
    FilePath b("/b");
    a.concat(b);

    std::string s = a.toString();
    assertEQ(s.size(), 3);
    assertEQ(s.at(0), 'a');
    assertEQ(s.at(1), FilePath::nativeSeparator());
    assertEQ(s.at(2), 'b');
    assert(!a.isAbsolute());
}

static void testFilePathConcat4() {
    FilePath a("a");
    FilePath b("./b");
    a.concat(b);

    std::string s = a.toString();
    assertEQ(s.size(), 3);
    assertEQ(s.at(0), 'a');
    assertEQ(s.at(1), FilePath::nativeSeparator());
    assertEQ(s.at(2), 'b');
    assert(!a.isAbsolute());
}

static void testFilePathConcat5() {
    FilePath a("a");
    FilePath b(".");
    a.concat(b);

    std::string s = a.toString();
    assertEQ(s.size(), 1);
    assertEQ(s.at(0), 'a');
    assert(!a.isAbsolute());
}

static void testFilePathConcat6() {
    FilePath a("");
    FilePath b("abc");
    a.concat(b);
    std::string s = a.toString();
    assertEQ(s, "abc");
    assert(!a.isAbsolute());
}

static void testFilePathGetPathPart() {
    FilePath a("abc/def\\ghi//a.txt");
    FilePath path = a.getPathPart();
    FilePath expected("abc\\def\\ghi\\");  // trailing separators don't really make a difference
    assertEQ(path.toString(), expected.toString());
    assert(!a.isAbsolute());
}

static void testFilePathGetPathPart2() {
    FilePath a("a.txt");
    FilePath path = a.getPathPart();
    FilePath expected("");
    assertEQ(expected.toString(), path.toString());
    assert(!a.isAbsolute());
}

static void testFilePathGetFilenamePart() {
    FilePath a("abc/def\\ghi//a.txt");
    std::string fileName = a.getFilenamePart();
    assertEQ(fileName, "a.txt");
    assert(!a.isAbsolute());
}

static void testFilePathGetFilenamePart2() {
    FilePath a("a.txt");
    std::string fileName = a.getFilenamePart();
    assertEQ(fileName, "a.txt");
    assert(!a.isAbsolute());
}

static void testFilePathGetFilenamePart3() {
    FilePath a("abc/");
    std::string fileName = a.getFilenamePart();
    assertEQ(fileName, "");
    assert(!a.isAbsolute());
}

static void testFilePathGetFilenamePartNoExtension() {
    FilePath a("abc/def.hij");
    std::string fileName = a.getFilenamePartNoExtension();
    assertEQ(fileName, "def");
    assert(!a.isAbsolute());
}

static void testFilePathGetFilenamePartNoExtension2() {
    FilePath a("abc/def.hij.klm");
    std::string fileName = a.getFilenamePartNoExtension();
    assertEQ(fileName, "def.hij");
    assert(!a.isAbsolute());
}

static void testFilePathGetFilenamePartNoExtension3() {
    FilePath a("abc/def");
    std::string fileName = a.getFilenamePartNoExtension();
    assertEQ(fileName, "def");
    assert(!a.isAbsolute());
}

static void testFilePathDoubleDot() {
    FilePath fp1("a");
    FilePath fp2("../b");
    fp1.concat(fp2);
    FilePath expected("a/../b");
    assertEQ(fp1.toString(), expected.toString());
    assert(!fp1.isAbsolute());
}

static void testFilePathExt() {
    FilePath fp1("a.b");
    assertEQ(fp1.getExtensionLC(), "b");

    FilePath fp2("a.b.c");
    assertEQ(fp2.getExtensionLC(), "c");

    FilePath fp3("a");
    assertEQ(fp3.getExtensionLC(), "");

    FilePath fp4("");
    assertEQ(fp4.getExtensionLC(), "");

    FilePath fp5("a.WAVeFILE");
    assertEQ(fp5.getExtensionLC(), "wavefile");
}

static void testFilePathAbs() {
    FilePath fp("c:\foo");
    assert(fp.isAbsolute());

    FilePath fp2("\\foo");
    assert(fp2.isAbsolute());
    SQINFO("fp2 is %s", fp2.toString().c_str());

    FilePath fp3("/foo");
    assert(fp3.isAbsolute());
}

static void testSchemaFreeText1() {
    bool b;
    b = SamplerSchema::isFreeTextType("foo");
    assert(!b);
    b = SamplerSchema::isFreeTextType("sample");
    assert(b);
    b = SamplerSchema::isFreeTextType("label_cc7");
    assert(b);
}

static void testSchemaTextBuiltIn() {
    // validate that we have all the known ones
    std::vector<std::string> known = SamplerSchema::_getKnownTextOpcodes();
    for (auto opcode : known) {
        assert(SamplerSchema::isFreeTextType(opcode));
    }

    std::vector<std::string> knownNot = SamplerSchema::_getKnownNonTextOpcodes();
    for (auto opcode : knownNot) {
        assert(!SamplerSchema::isFreeTextType(opcode));
    }
}

static void testSchemaIntPass() {
    int intVal = 0;
    bool b = SamplerSchema::stringToInt("10", &intVal);
    assert(b);
    assertEQ(intVal, 10);

    b = SamplerSchema::stringToInt("0", &intVal);
    assert(b);
    assertEQ(intVal, 0);

    b = SamplerSchema::stringToInt("123456789", &intVal);
    assert(b);
    assertEQ(intVal, 123456789);

    b = SamplerSchema::stringToInt("-123", &intVal);
    assert(b);
    assertEQ(intVal, -123);
}

static void testSchemaFloatPass() {
    float floatVal = 0;
    bool b = SamplerSchema::stringToFloat("10", &floatVal);
    assert(b);
    assertEQ(floatVal, 10);

    b = SamplerSchema::stringToFloat("0", &floatVal);
    assert(b);
    assertEQ(floatVal, 0);

    b = SamplerSchema::stringToFloat("123456789", &floatVal);
    assert(b);
    assertEQ(floatVal, 123456789);

    b = SamplerSchema::stringToFloat("-123", &floatVal);
    assert(b);
    assertEQ(floatVal, -123);

    b = SamplerSchema::stringToFloat("145.6", &floatVal);
    assert(b);
    assertClose(floatVal, 145.6, .00001);

    b = SamplerSchema::stringToFloat(".6", &floatVal);
    assert(b);
    assertClose(floatVal, .6, .00001);

    b = SamplerSchema::stringToFloat(".0", &floatVal);
    assert(b);
    assertEQ(floatVal, 0);
}

static void testSchemaIntFail() {
    int intVal = 1;
    bool b = SamplerSchema::stringToInt("bcd", &intVal);
    assert(!b);
    assertEQ(intVal, 0);

    b = SamplerSchema::stringToInt("123", nullptr);
    assert(!b);
    assertEQ(intVal, 0);

    b = SamplerSchema::stringToInt("-abc", &intVal);
    assert(!b);
    assertEQ(intVal, 0);

    b = SamplerSchema::stringToInt("a123", &intVal);
    assert(!b);
    assertEQ(intVal, 0);

    b = SamplerSchema::stringToInt("a", &intVal);
    assert(!b);
    assertEQ(intVal, 0);

    // Old version accepted this...
    b = SamplerSchema::stringToInt("123a", &intVal);
    assert(b);
    assertEQ(intVal, 123);
}

static void testSchemaFloatFail() {

    float floatVal = 1;
    bool b = SamplerSchema::stringToFloat("bcd", &floatVal);
    assert(!b);
    assertEQ(floatVal, 0);

    b = SamplerSchema::stringToFloat("123", nullptr);
    assert(!b);
    assertEQ(floatVal, 0);

    b = SamplerSchema::stringToFloat("-abc", &floatVal);
    assert(!b);
    assertEQ(floatVal, 0);

    b = SamplerSchema::stringToFloat("a123", &floatVal);
    assert(!b);
    assertEQ(floatVal, 0);

    b = SamplerSchema::stringToFloat("", &floatVal);
    assert(!b);
    assertEQ(floatVal, 0);

    // Old version accepted this...
    b = SamplerSchema::stringToFloat("123a", &floatVal);
    assert(b);
    assertEQ(floatVal, 123);
}


void testx4() {
    testFilePath0();
    testFilePathFixup();
    testFilePathFixup2();
    testFilePathConcat1();
    testFilePathConcat2();
    testFilePathConcat3();
    testFilePathConcat4();
    testFilePathConcat5();
    testFilePathConcat6();
    testFilePathGetPathPart();
    testFilePathGetPathPart2();
    testFilePathGetFilenamePart();
    testFilePathGetFilenamePart2();
    testFilePathGetFilenamePart3();
    testFilePathGetFilenamePartNoExtension();
    testFilePathGetFilenamePartNoExtension2();
    testFilePathDoubleDot();
    testFilePathExt();
    testFilePathAbs();

    testSchemaFreeText1();
    testSchemaTextBuiltIn();
    testSchemaIntPass();
    testSchemaIntFail();
    testSchemaFloatPass();
    testSchemaFloatFail();
}