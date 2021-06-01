

#include <set>

#include "FilePath.h"
#include "RandomRange.h"
#include "SInstrument.h"
#include "SLex.h"
#include "SParse.h"
#include "SqLog.h"
#include "asserts.h"
//#include "pugixml.hpp"

#if 0
static void testx0() {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file("fale_path.xml");
    auto status = result.status;
    int x = 5;
    assertEQ(status, pugi::xml_parse_status::status_file_not_found);
}
#endif

static void testx1() {
    SLexPtr lex = SLex::go("<global>");
    assert(lex);
    lex->validate();
    assertEQ(lex->items.size(), 1);
    assert(lex->items[0]->itemType == SLexItem::Type::Tag);
    SLexTag* ptag = static_cast<SLexTag*>(lex->items[0].get());
    assertEQ(ptag->tagName, "global");
}

static void testx2() {
    SLexPtr lex = SLex::go("=");
    assert(lex);
    lex->validate();
    assertEQ(lex->items.size(), 1);
    assert(lex->items[0]->itemType == SLexItem::Type::Equal);
}

static void testx3() {
    SLexPtr lex = SLex::go("qrst");
    assert(lex);
    lex->validate();
    assertEQ(lex->items.size(), 1);
    assert(lex->items[0]->itemType == SLexItem::Type::Identifier);
    SLexIdentifier* pid = static_cast<SLexIdentifier*>(lex->items[0].get());
    assertEQ(pid->idName, "qrst");
}

static void testxKVP() {
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

static void testxKVP2() {
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

#include <fstream>
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
    std::string err;
    auto lex = SLex::go(str, &err);
    assert(!lex);
    assert(!err.empty());
    assertEQ(err, "Malformed #include at line 1");
}

static void testLexIncludeBadFile() {
    std::string str("#include \"abc\"");
    std::string err;
    FilePath fp("fake");
    auto lex = SLex::go(str, &err, 0, &fp);

    assert(!err.empty());
    assertEQ(err, "Can't open abc included at line 1");

    // this should error out, as "abc" can't be opened.
    assert(!lex);
}

// This uses a "real" sfz on disk
static void testLexIncludeSuccess() {
    FilePath filePath(R"foo(D:\samples\test\test-include.sfz)foo");
    std::ifstream t(filePath.toString());
    assert(t.good());

    std::string sContent((std::istreambuf_iterator<char>(t)),
                         std::istreambuf_iterator<char>());

    std::string err;
    auto lex = SLex::go(sContent, &err, 0, &filePath);

    assert(lex && err.empty());

    assertEQ(lex->items.size(), 3);
    assertEQ(int(lex->items[0]->itemType), int(SLexItem::Type::Tag));
    assertEQ(int(lex->items[1]->itemType), int(SLexItem::Type::Tag));
    assertEQ(int(lex->items[2]->itemType), int(SLexItem::Type::Tag));
}

// can we parse a simple define?
static void testLexDefineSuccess() {
    std::string content(R"foo(#define A 22)foo");
    std::string err;
    auto lex = SLex::go(content, &err, 0);

    assert(lex && err.empty());
    assertEQ(lex->items.size(), 0);
}

static void testLexDefineSuccess2() {
    std::string content(R"foo(a=b #define A 22 c=d)foo");
    std::string err;
    auto lex = SLex::go(content, &err, 0);

    assert(lex && err.empty());
    assertEQ(lex->items.size(), 6);
}

static void testLexDefineSuccess3() {
    std::string content(R"foo(
<control>
default_path=Soft String Spurs Samples/

label_cc$MW=MW ($MW)
)foo");
    std::string err;
    auto lex = SLex::go(content, &err, 0);

    assert(lex && err.empty());
    assertEQ(lex->items.size(), 7);
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
    lex->_dump();
    assertEQ(lex->items.size(), 3);
    assert(lex->items[2]->itemType == SLexItem::Type::Identifier);
    SLexItemPtr id = lex->items[2];
    SLexIdentifier* p = static_cast<SLexIdentifier*>(id.get());
    assertEQ(p->idName, "a/b");
}

static void testLexCommentInFile2() {
    auto lex = SLex::go("sample=a/b //c");
    assert(lex);
    lex->_dump();
    assertEQ(lex->items.size(), 3);
    assert(lex->items[2]->itemType == SLexItem::Type::Identifier);
    SLexItemPtr id = lex->items[2];
    SLexIdentifier* p = static_cast<SLexIdentifier*>(id.get());
    assertEQ(p->idName, "a/b");
}

static void testLexCommentInFile3() {
    auto lex = SLex::go("sample=a/b\t//c");
    assert(lex);
    lex->_dump();
    assertEQ(lex->items.size(), 3);
    assert(lex->items[2]->itemType == SLexItem::Type::Identifier);
    SLexItemPtr id = lex->items[2];
    SLexIdentifier* p = static_cast<SLexIdentifier*>(id.get());
    assertEQ(p->idName, "a/b");
}
//
static void testLexMacPath() {
    auto lex = SLex::go("sample=/abs/path.wav");
    assert(lex);
    assertEQ(lex->items.size(), 3);
    assert(lex->items[2]->itemType == SLexItem::Type::Identifier);
    SLexIdentifier* ident = static_cast<SLexIdentifier*>(lex->items[2].get());
    assertEQ(ident->idName, "/abs/path.wav");
}

static void testparse1() {
    SInstrumentPtr inst = std::make_shared<SInstrument>();

    auto err = SParse::go("random-text", inst);
    assert(!err.empty());
}

static void testParseRegion() {
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("<region>", inst);
    assert(err.empty());

    assertEQ(inst->headings.size(), 1)
    assertEQ(int(inst->headings[0]->type), int(SHeading::Type::Region));
   // assertEQ(inst->groups.size(), 1);
   // assertEQ(inst->groups[0]->regions.size(), 1);
}



static void testparse2() {
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("<region>pitch_keycenter=24", inst);
    assert(err.empty());

    assertEQ(inst->headings.size(), 1);

    SHeadingPtr region = inst->headings[0];
    assertEQ(int(region->type), int(SHeading::Type::Region));

  //  assertEQ(inst->groups.size(), 1);
 //   SGroupPtr group = inst->groups[0];
 //   assert(group->values.empty());
 //  assertEQ(group->regions.size(), 1);
 //   SRegionPtr region = group->regions[0];
  //  assertEQ(region->values.size(), 1);
    SKeyValuePairPtr kv = region->values[0];
    assertEQ(kv->key, "pitch_keycenter");
    assertEQ(kv->value, "24");
}

// this sfz doesn't start with a heading,
// we currently give a terrible error message
static void testParseLabel2() {
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("label_cc7=Master Vol\nsample=abc", inst);
    assert(!err.empty());
    assertEQ(err.find("extra tok"), 0);
}


#if 0
static void testParseMutliControls() {
    // SQINFO("--- start testParseMutliControls");

    const char* test = R"foo(
        <control>
        default_path=a
        <region>
        sample=r1
        <control>
        default_path=b
        <region>
        sample=r2
    )foo";

    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go(test, inst);
    assert(err.empty());

    // we should end up with two regions in two groups
    // we always make a new group when we hit a new heading,
    // in this case <control>
    assertEQ(inst->groups.size(), 2);
    assertEQ(inst->groups[0]->regions.size(), 1);
    assertEQ(inst->groups[1]->regions.size(), 1);

    // each regions should have the 'sample' key, as well as the 'default_path' key

    SKeyValueList kv = inst->groups[0]->regions[0]->values;
    assertEQ(kv.size(), 2);

    assertEQ(kv[0]->key, "default_path");
    assertEQ(kv[0]->value, "a");
    assertEQ(kv[1]->key, "sample");
    assertEQ(kv[1]->value, "r1");
    //
    kv = inst->groups[1]->regions[0]->values;
    assertEQ(kv.size(), 2);
    assertEQ(kv[0]->key, "default_path");
    assertEQ(kv[0]->value, "b");
    assertEQ(kv[1]->key, "sample");
    assertEQ(kv[1]->value, "r2");
}
#endif


static void testParseGroupAndValues() {
    //SQINFO("---- start testParseGroupAndValues");
    //   const char* test = R"foo(<group>sample=K18\C7.pp.wav lovel=1 hivel=22 lokey=95 hikey=97 pitch_keycenter=96 tune=10 offset=200<region>)foo";
    const char* test = R"foo(<group>a=b<region>)foo";
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go(test, inst);
    assert(err.empty());

    assertEQ(inst->headings.size(), 2);
    assertEQ((int)inst->headings[0]->type, (int)SHeading::Type::Group);
    assertEQ((int)inst->headings[1]->type, (int)SHeading::Type::Region);

    assertEQ(inst->headings[0]->values.size(), 1);
    assertEQ(inst->headings[1]->values.size(), 0);
}


static void testParseGlobal() {
    // SQINFO("---- start test parse global\n");
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("<global>", inst);
    // no regions - that's not legal, but we make up groups if there aren't any,
    // so we don't consider it an error.
    assert(err.empty());
    assertEQ(inst->headings.size(), 1);
    assertEQ(int(inst->headings[0]->type), int(SHeading::Type::Global));
    //assert(inst->groups[0]->regions.empty());
}



static void testParseGlobalGroupAndRegion() {
    // SQINFO("\n-- start testParseGlobalAndRegion\n");
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("<global><group><region>", inst);

    assert(err.empty());
}

static void testParseGlobalAndRegion() {
    //  SQINFO("\n-- start testParseGlobalAndRegion\n");
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("<global><region>", inst);

    assert(err.empty());
}

static void testParseComment() {
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("// comment\n<global><region>", inst);
    assert(err.empty());
}




static void testParseGroups() {
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("<group><region><region>", inst);
    assert(err.empty());
}


static void testParseTwoGroupsA() {
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("<group><group>", inst);
    assert(err.empty());
    assertEQ(inst->headings.size(), 2);
    assertEQ((int)inst->headings[0]->type, (int)SHeading::Type::Group);
    assertEQ((int)inst->headings[1]->type, (int)SHeading::Type::Group);
}

static void testParseTwoGroupsB() {
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("<group><region><region><group><region>", inst);
    assert(err.empty());
    assertEQ(inst->headings.size(), 5);
    assertEQ((int)inst->headings[0]->type, (int)SHeading::Type::Group);
    assertEQ((int)inst->headings[1]->type, (int)SHeading::Type::Region);
    assertEQ((int)inst->headings[2]->type, (int)SHeading::Type::Region);
    assertEQ((int)inst->headings[3]->type, (int)SHeading::Type::Group);
    assertEQ((int)inst->headings[4]->type, (int)SHeading::Type::Region);
}

static void testParseGlobalWithData() {
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("<global>ampeg_release=0.6<region>", inst);
    assert(err.empty());
}


static void testparse_piano1() {
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    const char* p = R"foo(D:\samples\UprightPianoKW-small-SFZ-20190703\UprightPianoKW-small-20190703.sfz)foo";
    auto err = SParse::goFile(FilePath(p), inst);
    assert(err.empty());
}

static void testparse_piano2() {
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    const char* p = R"foo(D:\samples\k18-Upright-Piano\k18-Upright-Piano.sfz)foo";
    auto err = SParse::goFile(FilePath(p), inst);
    assert(err.empty());
}

static void testParseSimpleDrum() {
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    const char* p = R"foo(
        //snare =====================================
        <group> amp_veltrack=98 key=40 loop_mode=one_shot lovel=101 hivel=127  // snare1 /////
        <region> sample=a lorand=0 hirand=0.3
        <region> sample=b lorand=0.3 hirand=0.6
        <region> sample=c lorand=0.6 hirand=1.0

        //snareStick =====================================
        <group> amp_veltrack=98 volume=-11 key=41 loop_mode=one_shot lovel=1 hivel=127 seq_length=3 
        <region> sample=d seq_position=1
        <region> sample=e seq_position=2
        <region> sample=f seq_position=3
    )foo";

    auto err = SParse::go(p, inst);
    assert(err.empty());
    assertEQ(inst->headings.size(), 8);
    assertEQ((int)inst->headings[0]->type, (int)SHeading::Type::Group);
    assertEQ((int)inst->headings[1]->type, (int)SHeading::Type::Region);
    assertEQ((int)inst->headings[2]->type, (int)SHeading::Type::Region);
    assertEQ((int)inst->headings[3]->type, (int)SHeading::Type::Region);
    assertEQ((int)inst->headings[4]->type, (int)SHeading::Type::Group);
    assertEQ((int)inst->headings[5]->type, (int)SHeading::Type::Region);
    assertEQ((int)inst->headings[6]->type, (int)SHeading::Type::Region);
    assertEQ((int)inst->headings[7]->type, (int)SHeading::Type::Region);
}



// make sure we dont' crash from parsing unused regions.
static void testParseCurve() {
    const char* p = R"foo(
//--------------------------
<curve>
curve_index=7
v000=0
v001=1
v127=1

 )foo";
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go(p, inst);
    assert(err.empty());

    assertEQ(inst->headings.size(), 1);
    assertEQ((int)inst->headings[0]->type, (int)SHeading::Type::Curve);
}

static void testRandomRange0() {
    RandomRange<float> r(0);
    r.addRange(.33f);
    r.addRange(.66f);
    assertEQ(r._lookup(0), 0);
    assertEQ(r._lookup(.2f), 0);
    assertEQ(r._lookup(.32f), 0);
    assertEQ(r._lookup(.34f), 1);
    assertEQ(r._lookup(.659f), 1);
    assertEQ(r._lookup(.661f), 2);
    assertEQ(r._lookup(.9f), 2);
    assertEQ(r._lookup(10.f), 2);
}

static void testRandomRange1() {
    RandomRange<float> r(0);
    r.addRange(.3f);
    r.addRange(.4f);

    std::set<int> test;
    for (int i = 0; i < 50; ++i) {
        int x = r.get();
        test.insert(x);
    }
    assertEQ(test.size(), 3);
}


#if 0 // deleted this file

static void testParseDX() {
    SInstrumentPtr inst = std::make_shared<SInstrument>();

    // maybe need to compile this...
    auto err = SParse::goFile(FilePath("D:\\samples\\__test\\BS-DX7-Bright-Bow.sfz"), inst);
    assert(err.empty());

    assertEQ(inst->headings.size(), 14);
    assertEQ((int)inst->headings[0]->type, (int)SHeading::Type::Group);
    assertEQ((int)inst->headings[1]->type, (int)SHeading::Type::Region);
    assertEQ(inst->headings[0]->values.size(), 2);
    assertEQ(inst->headings[1]->values.size(), 5);
}
#endif


extern int compileCount;

#if 0
void testx3() {
    assert(false);
}
void testx4() {
    assert(false);
}

void testx5() {
    assert(false);
}

void testx6() {
    assert(false);
}
#endif

void testx() {
    assertEQ(compileCount, 0);
    assert(parseCount == 0);

    //testx0();
    testx1();
    testx2();
    testx3();
    testxKVP();
    testxKVP2();

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
    //  testLexDefineFail();
    testLexLabel2();
    testLexNewLine();
    testLexCommentInFile();
    testLexCommentInFile2();
    testLexCommentInFile3();
    testLexMacPath();

    testparse1();
    testParseRegion();
    testparse2();
    testParseGlobal();

   

    testParseGlobalAndRegion();
    testParseGlobalGroupAndRegion();

   

    testParseComment();
    testParseGroups();

    // better as a cimpiler test
  //  testParseMutliControls();
    testParseGroupAndValues();
    testParseLabel2();

    testParseGlobalWithData();
    testParseTwoGroupsA();
    testParseTwoGroupsB();
    testparse_piano1();

    // testparse_piano2b();
    testparse_piano2();
    testParseSimpleDrum();
    testRandomRange0();
    testRandomRange1();

    // merge conflict here. does this work? a: it was deleted in main
    //testParseDX();
    testParseCurve();
    assert(parseCount == 0);
}