#include "LexContext.h"
#include "SLex.h"
#include "FilePath.h"

#include "asserts.h"

#include <fstream>

static void testLex1() {
    SLexPtr lex = SLex::go("<global>");
    assert(lex);
    lex->validate();
    assertEQ(lex->items.size(), 1);
    assert(lex->items[0]->itemType == SLexItem::Type::Tag);
    SLexTag* ptag = static_cast<SLexTag*>(lex->items[0].get());
    assertEQ(ptag->tagName, "global");
}

static void testLex2() {
    SLexPtr lex = SLex::go("=");
    assert(lex);
    lex->validate();
    assertEQ(lex->items.size(), 1);
    assert(lex->items[0]->itemType == SLexItem::Type::Equal);
}

static void testLex3() {
    SLexPtr lex = SLex::go("qrst");
    assert(lex);
    lex->validate();
    assertEQ(lex->items.size(), 1);
    assert(lex->items[0]->itemType == SLexItem::Type::Identifier);
    SLexIdentifier* pid = static_cast<SLexIdentifier*>(lex->items[0].get());
    assertEQ(pid->idName, "qrst");
}

static void testLexKVP() {
    SLexPtr lex = SLex::go("abc=def");
    assert(lex);
    lex->validate();
    assertEQ(lex->items.size(), 3);
    assert(lex->items[0]->itemType == SLexItem::Type::Identifier);
    SLexIdentifier* pid = static_cast<SLexIdentifier*>(lex->items[0].get());
    assertEQ(pid->idName, "abc");

    assert(lex->items[1]->itemType == SLexItem::Type::Equal);

    assert(lex->items[2]->itemType == SLexItem::Type::Identifier);
    pid = static_cast<SLexIdentifier*>(lex->items[2].get());
    assertEQ(pid->idName, "def");
}

static void testLexKVP2() {
    SLexPtr lex = SLex::go("ampeg_release=0.6");
    assert(lex);
    lex->validate();
    assertEQ(lex->items.size(), 3);
    assert(lex->items[0]->itemType == SLexItem::Type::Identifier);
    SLexIdentifier* pid = static_cast<SLexIdentifier*>(lex->items[0].get());
    assertEQ(pid->idName, "ampeg_release");

    assert(lex->items[2]->itemType == SLexItem::Type::Identifier);
    pid = static_cast<SLexIdentifier*>(lex->items[2].get());
    assertEQ(pid->idName, "0.6");
}

static void testLexTrivialComment() {
    SLexPtr lex = SLex::go("//");
    assert(lex);
    lex->validate();
    assertEQ(lex->items.size(), 0);
}

static void testLexComment() {
    SLexPtr lex = SLex::go("// comment\n<global>");
    assert(lex);
    lex->validate();
    assertEQ(lex->items.size(), 1);
    assert(lex->items[0]->itemType == SLexItem::Type::Tag);
    SLexTag* pTag = static_cast<SLexTag*>(lex->items[0].get());
    assertEQ(pTag->tagName, "global");
    assertEQ(pTag->lineNumber, 1);
}

static void testLexComment2() {
    SLexPtr lex = SLex::go("// comment\n//comment\n\n<global>\n\n");
    assert(lex);
    lex->validate();
    assertEQ(lex->items.size(), 1);
    assert(lex->items[0]->itemType == SLexItem::Type::Tag);
    SLexTag* pTag = static_cast<SLexTag*>(lex->items[0].get());
    assertEQ(pTag->tagName, "global");
}

static void testLexMultiLineCommon(const char* data) {
    SLexPtr lex = SLex::go(data);
    assert(lex);
    lex->validate();
    assertEQ(lex->items.size(), 3);

    assert(lex->items[0]->itemType == SLexItem::Type::Tag);
    SLexTag* pTag = static_cast<SLexTag*>(lex->items[0].get());
    assertEQ(pTag->tagName, "one");
    assertEQ(pTag->lineNumber, 0);

    assert(lex->items[1]->itemType == SLexItem::Type::Tag);
    pTag = static_cast<SLexTag*>(lex->items[1].get());
    assertEQ(pTag->tagName, "two");
    assertEQ(pTag->lineNumber, 1);

    assert(lex->items[2]->itemType == SLexItem::Type::Tag);
    pTag = static_cast<SLexTag*>(lex->items[2].get());
    assertEQ(pTag->tagName, "three");
    assertEQ(pTag->lineNumber, 2);
}

static void testLexMultiLine1() {
    testLexMultiLineCommon("<one>\n<two>\n<three>");
}

static void testLexMultiLine2() {
    testLexMultiLineCommon(R"(<one>
    <two>
    <three>)");
}

static void testLexGlobalWithData() {
    SLexPtr lex = SLex::go("<global>ampeg_release=0.6<region>");
    assert(lex);
    lex->validate();
    assertEQ(lex->items.size(), 5);
    assert(lex->items.back()->itemType == SLexItem::Type::Tag);
    SLexTag* tag = static_cast<SLexTag*>(lex->items.back().get());
    assertEQ(tag->tagName, "region");
}

static void testLexTwoRegions() {
    SLexPtr lex = SLex::go("<region><region>");
    assert(lex);
    lex->validate();

    assertEQ(lex->items.size(), 2);
    assert(lex->items.back()->itemType == SLexItem::Type::Tag);
    SLexTag* tag = static_cast<SLexTag*>(lex->items.back().get());
    assertEQ(tag->tagName, "region");
}

static void testLexTwoKeys() {
    SLexPtr lex = SLex::go("a=b\nc=d");
    assert(lex);
    lex->validate();

    assertEQ(lex->items.size(), 6);
    assert(lex->items.back()->itemType == SLexItem::Type::Identifier);
    SLexIdentifier* id = static_cast<SLexIdentifier*>(lex->items.back().get());
    assertEQ(id->idName, "d");
}

static void testLexTwoKeysOneLine() {
    SLexPtr lex = SLex::go("a=b c=d");
    assert(lex);
    lex->validate();

    assertEQ(lex->items.size(), 6);
    assert(lex->items.back()->itemType == SLexItem::Type::Identifier);
    SLexIdentifier* id = static_cast<SLexIdentifier*>(lex->items.back().get());
    assertEQ(id->idName, "d");
}

static void testLexTwoRegionsWithKeys() {
    SLexPtr lex = SLex::go("<region>a=b\nc=d<region>q=w\ne=r");
    assert(lex);
    lex->validate();

    assertEQ(lex->items.size(), 14);
    assert(lex->items.back()->itemType == SLexItem::Type::Identifier);
    SLexIdentifier* id = static_cast<SLexIdentifier*>(lex->items.back().get());
    assertEQ(id->idName, "r");
}

static void testLexMangledId() {
    SLexPtr lex = SLex::go("<abd\ndef>");
    assert(!lex);
}

static void testLex4() {
    auto lex = SLex::go("<group><region><region><group><region.");
    assert(!lex);
}

static void testLex5() {
    auto lex = SLex::go("\n<group>");
    assert(lex);
    lex->validate();
    SLexTag* tag = static_cast<SLexTag*>(lex->items.back().get());
    assertEQ(tag->tagName, "group");
}

static void testLexSpaces() {
    auto lex = SLex::go("\nsample=a b c");
    assert(lex);
    lex->validate();
    SLexIdentifier* fname = static_cast<SLexIdentifier*>(lex->items.back().get());
    assertEQ(fname->idName, "a b c");
}

/**
 * tests lexing of things like "sample=foo a=b"
 * test string is expected to have a sample- and x=y
 */
static void testLexSpaces2Sub(const std::string& testString, const std::string& expectedFileName) {
    auto lex = SLex::go(testString);
    assert(lex);
    lex->validate();
    SLexIdentifier* lastid = static_cast<SLexIdentifier*>(lex->items.back().get());
    assertEQ(lastid->idName, "y");
    const auto num = lex->items.size();
    assert(lex->items[num - 2]->itemType == SLexItem::Type::Equal);
    assert(lex->items[num - 3]->itemType == SLexItem::Type::Identifier);
    SLexIdentifier* xident = static_cast<SLexIdentifier*>(lex->items[num - 3].get());
    assertEQ(xident->idName, "x");

    SLexIdentifier* fname = static_cast<SLexIdentifier*>(lex->items[num - 4].get());
    assertEQ(fname->idName, expectedFileName);
}

static void testLexSpaces2a() {
    testLexSpaces2Sub("sample=abc x=y", "abc");
}

static void testLexSpaces2b() {
    testLexSpaces2Sub("sample=abc  x=y", "abc");
}

static void testLexSpaces2c() {
    testLexSpaces2Sub("sample=a b c    x=y", "a b c");
}

static void testLexSpaces2d() {
    const char* pAllDrum = R"foo(sample=a
//comm
    x = y
)foo";
    testLexSpaces2Sub(pAllDrum, "a");
}
static void testLexSpaces2() {
    testLexSpaces2a();
    testLexSpaces2b();
    testLexSpaces2c();
    testLexSpaces2d();
}

static void testLexLabel() {
    std::string str("\nsw_label=abc def ghi");
    auto lex = SLex::go(str);
    assert(lex);
    lex->validate();
    SLexIdentifier* fname = static_cast<SLexIdentifier*>(lex->items.back().get());
    assertEQ(fname->idName, "abc def ghi");
}



static void testLexBeef() {
    // this real sfz has lots of includes.
    FilePath path("d:\\samples\\beefowulf_alpha_0100\\Programs\\beefowulf_keyswitch.sfz");
    std::ifstream t(path.toString());
    if (!t.good()) {
        printf("can't open file\n");
        assert(false);
        return;
    }
   

    std::string str((std::istreambuf_iterator<char>(t)),
                    std::istreambuf_iterator<char>());
    if (str.empty()) {
        assert(false);
        return;
    }

    LexContextPtr ctx = std::make_shared<LexContext>(str);
    ctx->addRootPath(path);
    auto lex = SLex::go(ctx);

    assert(lex);
    lex->validate();
    assertGT(lex->items.size(), 20000);
}

static void testLexMarimba2() {
    std::string path("d:\\samples\\test\\PatchArena_marimba.sfz");

    std::ifstream t(path);
    if (!t.good()) {
        printf("can't open file\n");
        assert(false);
        return;
    }

    std::string str((std::istreambuf_iterator<char>(t)),
                    std::istreambuf_iterator<char>());
    if (str.empty()) {
        assert(false);
        return;
    }

    auto lex = SLex::go(str);

    assert(lex);
    lex->validate();
    assertEQ(lex->items.size(), 1001);
    // assert(false);
}

static void testLexMarimba() {
    std::string str("<region> trigger=attack  pitch_keycenter=36 lokey=36 hikey=36 sample=PatchArena_marimba-036-c1.wav\r\n\r\n");
    auto lex = SLex::go(str);
    assert(lex);
    lex->validate();
    const size_t items = lex->items.size();
    assert(items == 16);
}

static void testLexIncludeMalformed() {
    std::string str("#includ \"abc\"");
    LexContextPtr ctx = std::make_shared<LexContext>(str);
    auto lex = SLex::go(ctx);
    assert(!lex);
    assert(!ctx->errorString().empty());
    assertEQ(ctx->errorString(), "Malformed #include at line 1");
}

static void testLexIncludeBadFile() {
    std::string str("#include \"abc\"");
    FilePath fp("fake");
    LexContextPtr ctx = std::make_shared<LexContext>(str);
    ctx->addRootPath(fp);
    auto lex = SLex::go(ctx);

    // this should error out, as "abc" can't be opened.
    assert(!ctx->errorString().empty());
    assert(!lex);
    assertEQ(ctx->errorString(), "Can't open abc included at line 1");
}

// This uses a "real" sfz on disk
static void testLexIncludeSuccess() {
    FilePath filePath(R"foo(D:\samples\test\test-include.sfz)foo");
    std::ifstream t(filePath.toString());
    assert(t.good());

    std::string sContent((std::istreambuf_iterator<char>(t)),
                         std::istreambuf_iterator<char>());

    LexContextPtr ctx = std::make_shared<LexContext>(sContent);
    ctx->addRootPath(filePath);
    auto lex = SLex::go(ctx);

    assert(lex && ctx->errorString().empty());

    assertEQ(lex->items.size(), 3);
    assertEQ(int(lex->items[0]->itemType), int(SLexItem::Type::Tag));
    assertEQ(int(lex->items[1]->itemType), int(SLexItem::Type::Tag));
    assertEQ(int(lex->items[2]->itemType), int(SLexItem::Type::Tag));
}

// can we parse a simple define?
static void testLexDefineSuccess() {
    std::string content(R"foo(#define A 22)foo");
    LexContextPtr ctx = std::make_shared<LexContext>(content);
    auto lex = SLex::go(ctx);

    assert(lex && ctx->errorString().empty());
    assertEQ(lex->items.size(), 0);
}

static void testLexDefineSuccess2() {
    std::string content(R"foo(a=b #define A 22 c=d)foo");
    LexContextPtr ctx = std::make_shared<LexContext>(content);
    auto lex = SLex::go(ctx);

    assert(lex && ctx->errorString().empty());
    assertEQ(lex->items.size(), 6);
}

static void testLexDefineSuccess3() {
    std::string content(R"foo(
<control>
default_path=Soft String Spurs Samples/

label_cc$MW=MW ($MW)
)foo");

    LexContextPtr ctx = std::make_shared<LexContext>(content);
    auto lex = SLex::go(ctx);

    assert(lex && ctx->errorString().empty());
    assertEQ(lex->items.size(), 7);
}

static void testLexDefineSuccess4() {
    std::string content(R"foo(#define $ABC 27
        label_cc$ABC=foo)foo");

    LexContextPtr ctx = std::make_shared<LexContext>(content);
    auto lex = SLex::go(ctx);

    assert(lex && ctx->errorString().empty());
    assertEQ(lex->items.size(), 3);
    auto label = lex->items[0];

    assert(lex->items[0]->itemType == SLexItem::Type::Identifier);
    SLexIdentifier* p = static_cast<SLexIdentifier*>(lex->items[0].get());
    std::string s = p->idName;
    assertEQ(s, "label_cc27");
}

// test that includes can find defines from parents.
static void testLexDefineNested() {
    std::string baseContent = R"foo(#define $ABC 27
        label_cc$ABC
        #include "stuff/a.sfz")foo";
    std::string includeContent = R"foo(label_cc$ABC)foo";

    LexContextPtr ctx = std::make_shared<LexContext>(baseContent);
    FilePath root("c:/files/x.sfz");
    ctx->addRootPath(root);
    ctx->addTestFolder(FilePath("c:/files/stuff/a.sfz"), includeContent);
    auto lex = SLex::go(ctx);
    assert(lex && ctx->errorString().empty());

    assertEQ(lex->items.size(), 2);
    assert(lex->items[0]->itemType == SLexItem::Type::Identifier);
    assert(lex->items[1]->itemType == SLexItem::Type::Identifier);

    SLexIdentifier* p = static_cast<SLexIdentifier*>(lex->items[0].get());
    std::string s = p->idName;
    assertEQ(s, "label_cc27");
    p = static_cast<SLexIdentifier*>(lex->items[1].get());
    s = p->idName;
    assertEQ(s, "label_cc27");
}

// test that includes prefer local overrides
static void testLexDefineNested2() {
    std::string baseContent = R"foo(#define $ABC 27
        #include "stuff/a.sfz")foo";
    std::string includeContent = R"foo(#define $ABC 72 label_cc$ABC)foo";
 
    LexContextPtr ctx = std::make_shared<LexContext>(baseContent);
    FilePath root("c:/files/x.sfz");
    ctx->addRootPath(root);
    ctx->addTestFolder(FilePath("c:/files/stuff/a.sfz"), includeContent);
    auto lex = SLex::go(ctx);
    assert(lex && ctx->errorString().empty());

    assertEQ(lex->items.size(), 1);
    assert(lex->items[0]->itemType == SLexItem::Type::Identifier);
    SLexIdentifier* p = static_cast<SLexIdentifier*>(lex->items[0].get());
    std::string s = p->idName;
    assertEQ(s, "label_cc72");
}

// test that defines in includes go away
// when they leave scope
static void testLexDefineNested3() {

    std::string baseContent = R"foo(#define $ABC 27
            #define $ABC 22
            #include "stuff/a.sfz"
            label_cc$ABC
        )foo";
    std::string includeContent = R"foo(#define $ABC 72)foo";

    LexContextPtr ctx = std::make_shared<LexContext>(baseContent);
    FilePath root("c:/files/x.sfz");
    ctx->addRootPath(root);
    ctx->addTestFolder(FilePath("c:/files/stuff/a.sfz"), includeContent);
    auto lex = SLex::go(ctx);
    assert(lex && ctx->errorString().empty());

    assertEQ(lex->items.size(), 1);
    assert(lex->items[0]->itemType == SLexItem::Type::Identifier);
    SLexIdentifier* p = static_cast<SLexIdentifier*>(lex->items[0].get());
    std::string s = p->idName;
    assertEQ(s, "label_cc22");
}

#if 0
static void testLexDefineFail() {
    std::string content(R"foo(#define A x=y)foo");
    std::string err;
    
    auto lex = SLex::go(content, &err, 0);

    assert(!lex && !err.empty());

}
#endif

static void testLexLabel2() {
    auto lex = SLex::go("label_cc7=Master Vol\nsample=\"abc def\"");
    assert(lex);
    assertEQ(lex->items.size(), 6);
}

static void testLexNewLine() {
    auto lex = SLex::go("sample=BS DX7 Bright Bow-000-084-c5.wav\r\n");
    assert(lex);
    assertEQ(lex->items.size(), 3);
}

static void testLexCommentInFile() {
    auto lex = SLex::go("sample=a/b//c");
    assert(lex);
    // lex->_dump();
    assertEQ(lex->items.size(), 3);
    assert(lex->items[2]->itemType == SLexItem::Type::Identifier);
    SLexItemPtr id = lex->items[2];
    SLexIdentifier* p = static_cast<SLexIdentifier*>(id.get());
    assertEQ(p->idName, "a/b");
}

static void testLexCommentInFile2() {
    auto lex = SLex::go("sample=a/b //c");
    assert(lex);
    //lex->_dump();
    assertEQ(lex->items.size(), 3);
    assert(lex->items[2]->itemType == SLexItem::Type::Identifier);
    SLexItemPtr id = lex->items[2];
    SLexIdentifier* p = static_cast<SLexIdentifier*>(id.get());
    assertEQ(p->idName, "a/b");
}

static void testLexCommentInFile3() {
    auto lex = SLex::go("sample=a/b\t//c");
    assert(lex);
    //lex->_dump();
    assertEQ(lex->items.size(), 3);
    assert(lex->items[2]->itemType == SLexItem::Type::Identifier);
    SLexItemPtr id = lex->items[2];
    SLexIdentifier* p = static_cast<SLexIdentifier*>(id.get());
    assertEQ(p->idName, "a/b");
}

static void testLexMacPath() {
    auto lex = SLex::go("sample=/abs/path.wav");
    assert(lex);
    assertEQ(lex->items.size(), 3);
    assert(lex->items[2]->itemType == SLexItem::Type::Identifier);
    SLexIdentifier* ident = static_cast<SLexIdentifier*>(lex->items[2].get());
    assertEQ(ident->idName, "/abs/path.wav");
}

static void testLexPathTrailingSpace() {
    auto lex = SLex::go("sample=/abs/path.wav ");
    assert(lex);
    assertEQ(lex->items.size(), 3);
    assert(lex->items[2]->itemType == SLexItem::Type::Identifier);
    SLexIdentifier* ident = static_cast<SLexIdentifier*>(lex->items[2].get());
    assertEQ(ident->idName, "/abs/path.wav");
}



void testxLex() {
    testLex1();
    testLex2();
    testLex3();
    testLexKVP();
    testLexKVP2();

    testLexTrivialComment();
    testLexComment();
    testLexComment2();
    testLexMultiLine1();
    testLexMultiLine2();
    testLexGlobalWithData();
    testLexTwoRegions();
    testLexTwoKeys();
    testLexTwoKeysOneLine();
    testLexTwoRegionsWithKeys();
    testLexMangledId();
    testLex4();
    testLex5();
    testLexSpaces();
    testLexSpaces2();
    testLexLabel();
    testLexMarimba();
    testLexIncludeMalformed();
    testLexIncludeBadFile();
    testLexIncludeSuccess();
    testLexDefineSuccess();
    testLexDefineSuccess2();
    testLexDefineSuccess3();
    testLexDefineSuccess4();
    testLexDefineNested();
    testLexDefineNested2();
    testLexDefineNested3();
    //  testLexDefineFail();
    testLexLabel2();
    testLexNewLine();
    testLexCommentInFile();
    testLexCommentInFile2();
    testLexCommentInFile3();
    testLexMacPath();
    testLexPathTrailingSpace();
    testLexBeef();


}