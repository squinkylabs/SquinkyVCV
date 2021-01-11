

#include <set>

#include "RandomRange.h"
#include "SInstrument.h"
#include "SLex.h"
#include "SParse.h"
#include "SqLog.h"
#include "asserts.h"
#include "pugixml.hpp"

static void testx0() {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file("fale_path.xml");
    auto status = result.status;
    int x = 5;
    assertEQ(status, pugi::xml_parse_status::status_file_not_found);
}

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

static void testLexComment() {
    printf("\n---- testLexComment\n");
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
    printf("testParseGlobalWithData\n");
    SLexPtr lex = SLex::go("<global>ampeg_release=0.6<region>");
    assert(lex);
    lex->validate();
    assertEQ(lex->items.size(), 5);
    assert(lex->items.back()->itemType == SLexItem::Type::Tag);
    SLexTag* tag = static_cast<SLexTag*>(lex->items.back().get());
    assertEQ(tag->tagName, "region");
}

static void testLexTwoRegions() {
    printf("testLexTwoRegions\n");
    SLexPtr lex = SLex::go("<region><region>");
    assert(lex);
    lex->validate();

    assertEQ(lex->items.size(), 2);
    assert(lex->items.back()->itemType == SLexItem::Type::Tag);
    SLexTag* tag = static_cast<SLexTag*>(lex->items.back().get());
    assertEQ(tag->tagName, "region");
}

static void testLexTwoKeys() {
    printf("testLexTwoRegionsValues\n");
    SLexPtr lex = SLex::go("a=b\nc=d");
    assert(lex);
    lex->validate();
    //lex->_dump();

    assertEQ(lex->items.size(), 6);
    assert(lex->items.back()->itemType == SLexItem::Type::Identifier);
    SLexIdentifier* id = static_cast<SLexIdentifier*>(lex->items.back().get());
    assertEQ(id->idName, "d");
}

static void testLexTwoKeysOneLine() {
    SLexPtr lex = SLex::go("a=b c=d");
    assert(lex);
    lex->validate();
    //lex->_dump();

    assertEQ(lex->items.size(), 6);
    assert(lex->items.back()->itemType == SLexItem::Type::Identifier);
    SLexIdentifier* id = static_cast<SLexIdentifier*>(lex->items.back().get());
    assertEQ(id->idName, "d");
}

static void testLexTwoRegionsWithKeys() {
    printf("testLexTwoRegionsValues\n");
    SLexPtr lex = SLex::go("<region>a=b\nc=d<region>q=w\ne=r");
    assert(lex);
    lex->validate();
    //lex->_dump();

    assertEQ(lex->items.size(), 14);
    assert(lex->items.back()->itemType == SLexItem::Type::Identifier);
    SLexIdentifier* id = static_cast<SLexIdentifier*>(lex->items.back().get());
    assertEQ(id->idName, "r");
}

static void testLexMangledId() {
    SLexPtr lex = SLex::go("<abd\ndef>");
    // if (lex) lex->_dump();
    assert(!lex);
}

static void testLex4() {
    printf("\ntestLex5s\n");
    auto lex = SLex::go("<group><region><region><group><region.");
    assert(!lex);
}

static void testLex5() {
    printf("\ntestLex5\n");
    auto lex = SLex::go("\n<group>");
    assert(lex);
    lex->validate();
    SLexTag* tag = static_cast<SLexTag*>(lex->items.back().get());
    assertEQ(tag->tagName, "group");
}

static void testLexSpaces() {
    SQWARN("staring test lex spaces");
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
    SQWARN("staring test lex spaces 2");
    testLexSpaces2Sub("sample=abc x=y", "abc");
}

static void testLexSpaces2b() {
    SQWARN("staring test lex spaces 2b");
    testLexSpaces2Sub("sample=abc  x=y", "abc");
}

static void testLexSpaces2c() {
    SQWARN("staring test lex spaces 2b");
    testLexSpaces2Sub("sample=a b c    x=y", "a b c");
}

static void testLexSpaces2d() {
    SQWARN("staring test lex spaces 2D");
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

#if 0
// get rid of this test. covered in case d
static void testAllDrum()
{
    const char* pAllDrum = R"foo(sample=a
//comm
)foo";

    SQINFO("test is %s", pAllDrum);
    auto lex = SLex::go(pAllDrum);
    assert(lex);
    lex->_dump();

    SLexIdentifier* ident = static_cast<SLexIdentifier*>(lex->items.back().get());
    assertEQ(ident->idName, "a");
    assert(false);
}
#endif



static void testparse1() {
    SInstrumentPtr inst = std::make_shared<SInstrument>();

    auto err = SParse::go("random-text", inst);
    assert(!err.empty());
}

static void testparse2() {
    printf("\nstart testprse2\n");
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("<region>pitch_keycenter=24", inst);
    assert(err.empty());
    ;
    assertEQ(inst->groups.size(), 1);
    SGroupPtr group = inst->groups[0];
    assert(group->values.empty());
    assertEQ(group->regions.size(), 1);
    SRegionPtr region = group->regions[0];
    assertEQ(region->values.size(), 1);
    SKeyValuePairPtr kv = region->values[0];
    assertEQ(kv->key, "pitch_keycenter");
    assertEQ(kv->value, "24");
}

static void testParseGlobal() {
    printf("start test parse global\n");
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("<global>", inst);
    // no regions - that's not legal
    assert(!err.empty());
}

static void testParseGlobalAndRegion() {
    printf("start test parse global\n");
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
    printf("testParseGroups\n");
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("<group><region><region>", inst);
    assert(err.empty());
}

static void testParseTwoGroupsA() {
    printf("\ntestParseTwoGroups\n");
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("<group><group>", inst);
    assert(err.empty());
    assertEQ(inst->groups.size(), 2);
}

static void testParseTwoGroupsB() {
    printf("\ntestParseTwoGroups\n");
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("<group><region><region><group><region>", inst);
    assert(err.empty());
    assertEQ(inst->groups.size(), 2);
}

static void testParseGlobalWithData() {
    printf("testParseGlobalWithData\n");
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("<global>ampeg_release=0.6<region>", inst);
    assert(err.empty());
}

static void testparse_piano1() {
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    const char* p = R"foo(D:\samples\UprightPianoKW-small-SFZ-20190703\UprightPianoKW-small-20190703.sfz)foo";
    printf("p=%s\n", p);
    auto err = SParse::goFile(p, inst);
    assert(err.empty());
}

static void testparse_piano2() {
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    const char* p = R"foo(D:\samples\k18-Upright-Piano\k18-Upright-Piano.sfz)foo";
    printf("p=%s\n", p);
    auto err = SParse::goFile(p, inst);
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
    assertEQ(inst->groups.size(), 2);
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

extern int compileCount;

void testx() {
    assertEQ(compileCount, 0);
    assert(parseCount == 0);

    testx0();
    testx1();
    testx2();
    testx3();
    testxKVP();
    testxKVP2();
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

    testparse1();
    testparse2();
    testParseGlobal();
    testParseGlobalAndRegion();
    testParseComment();
    testParseGroups();

    testParseGlobalWithData();
    testParseTwoGroupsA();
    testParseTwoGroupsB();
    testparse_piano1();
    // testparse_piano2b();
    testparse_piano2();
    testParseSimpleDrum();
    testRandomRange0();
    testRandomRange1();
    assert(parseCount == 0);
}