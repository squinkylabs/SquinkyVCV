

#include "pugixml.hpp"
#include "SLex.h"
#include "SParse.h"

#include "asserts.h"

static void testx0()
{
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file("fale_path.xml");
    auto status = result.status;
    int x = 5;
    assertEQ(status, pugi::xml_parse_status::status_file_not_found);
}


static void testx1()
{
    SLexPtr lex = SLex::go("<global>");
    assert(lex);
    lex->validate();
    assertEQ(lex->items.size(), 1);
    assert(lex->items[0]->itemType == SLexItem::Type::Tag);
    SLexTag* ptag = static_cast<SLexTag *>(lex->items[0].get());
    assertEQ(ptag->tagName, "global");
}

static void testx2()
{
    SLexPtr lex = SLex::go("=");
    assert(lex);
    lex->validate();
    assertEQ(lex->items.size(), 1);
    assert(lex->items[0]->itemType == SLexItem::Type::Equal);
}


static void testx3()
{
    SLexPtr lex = SLex::go("qrst");
    assert(lex);
    lex->validate();
    assertEQ(lex->items.size(), 1);
    assert(lex->items[0]->itemType == SLexItem::Type::Identifier);
    SLexIdentifier* pid = static_cast<SLexIdentifier *>(lex->items[0].get());
    assertEQ(pid->idName, "qrst");
}

static void testxKVP()
{
    SLexPtr lex = SLex::go("abc=def");
    assert(lex);
    lex->validate();
    assertEQ(lex->items.size(), 3);
    assert(lex->items[0]->itemType == SLexItem::Type::Identifier);
    SLexIdentifier* pid = static_cast<SLexIdentifier *>(lex->items[0].get());
    assertEQ(pid->idName, "abc");

    assert(lex->items[1]->itemType == SLexItem::Type::Equal);

    assert(lex->items[2]->itemType == SLexItem::Type::Identifier);
    pid = static_cast<SLexIdentifier*>(lex->items[2].get());
    assertEQ(pid->idName, "def");
}


static void testxKVP2()
{
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

static void testLexComment()
{
    SLexPtr lex = SLex::go("// comment\n<global>");
    assert(lex);
    lex->validate();
    assertEQ(lex->items.size(), 1);
    assert(lex->items[0]->itemType == SLexItem::Type::Tag);
    SLexTag* pTag = static_cast<SLexTag*>(lex->items[0].get());
    assertEQ(pTag->tagName, "global")
}


static void testLexComment2()
{
    SLexPtr lex = SLex::go("// comment\n//comment\n\n<global>\n\n");
    assert(lex);
    lex->validate();
    assertEQ(lex->items.size(), 1);
    assert(lex->items[0]->itemType == SLexItem::Type::Tag);
    SLexTag* pTag = static_cast<SLexTag*>(lex->items[0].get());
    assertEQ(pTag->tagName, "global")
}

static void testLexGlobalWithData()
{
    printf("testParseGlobalWithData\n");
    SLexPtr lex = SLex::go("<global>ampeg_release=0.6<region>");
    assert(lex);
    lex->validate();
    assertEQ(lex->items.size(), 5);
    assert(lex->items.back()->itemType == SLexItem::Type::Tag);
    SLexTag* tag = static_cast<SLexTag*>(lex->items.back().get());
    assertEQ(tag->tagName, "region");
}

static void testLexTwoRegions()
{
    printf("testLexTwoRegions\n");
    SLexPtr lex = SLex::go("<region><region>");
    assert(lex);
    lex->validate();

    assertEQ(lex->items.size(), 2);
    assert(lex->items.back()->itemType == SLexItem::Type::Tag);
    SLexTag* tag = static_cast<SLexTag*>(lex->items.back().get());
    assertEQ(tag->tagName, "region");
}


static void testLexTwoKeys()
{
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

static void testLexTwoRegionsWithKeys()
{
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

static void testLexMangledId()
{
    SLexPtr lex = SLex::go("<abd\ndef>");
   // if (lex) lex->_dump();
    assert(!lex);
}

static void testLex4()
{
    printf("\ntestLex5s\n");
    auto lex = SLex::go("<group><region><region><group><region.");
    assert(!lex);
}

static void testLex5()
{
    printf("\ntestLex5\n");
    auto lex = SLex::go("\n<group>");
    assert(lex);
    lex->validate();
    SLexTag* tag = static_cast<SLexTag*>(lex->items.back().get());
    assertEQ(tag->tagName, "group");
}

static void testparse1()
{
    SInstrumentPtr inst = std::make_shared<SInstrument>();

    auto err = SParse::go("random-text", inst);
    assert(!err.empty());
}

static void testparse2()
{
    printf("\nstart testprse2\n");
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("<region>pitch_keycenter=24", inst);
    assert(err.empty());;
    assertEQ(inst->groups.size(), 1);
    SGroupPtr group = inst->groups[0];
    assert(group->values.empty());
    assertEQ(group->regions.size(), 1);
    SRegionPtr region = group->regions[0];
    assertEQ(region->values.size(), 1);
    SKeyValuePairPtr kv = region->values[0];
    assertEQ(kv->key, "pitch_keycenter");
    assertEQ(kv->value , "24");
}

static void testParseGlobal()
{
    printf("start test parse global\n");
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("<global>", inst);
    // no regions - that's not legal
    assert(!err.empty());
}

static void testParseGlobalAndRegion()
{
    printf("start test parse global\n");
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("<global><region>", inst);

    assert(err.empty());
}

static void testParseComment()
{
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("// comment\n<global><region>", inst);
    assert(err.empty());
}

static void testParseGroups()
{
    printf("testParseGroups\n");
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("<group><region><region>", inst);
    assert(err.empty());
}

static void testParseTwoGroupsA()
{
    printf("\ntestParseTwoGroups\n");
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("<group><group>", inst);
    assert(err.empty());
    assertEQ(inst->groups.size(), 2);
}

static void testParseTwoGroupsB()
{
    printf("\ntestParseTwoGroups\n");
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("<group><region><region><group><region>", inst);
    assert(err.empty());
    assertEQ(inst->groups.size(), 2);
}

static void testParseGlobalWithData()
{
    printf("testParseGlobalWithData\n");
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("<global>ampeg_release=0.6<region>", inst);
    assert(err.empty());
}

static void testparse_piano1()
{
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    const char* p = R"foo(D:\samples\UprightPianoKW-small-SFZ-20190703\UprightPianoKW-small-20190703.sfz)foo";
    printf("p=%s\n", p);
    auto err = SParse::goFile(p, inst);
    assert(err.empty());
}

void testx()
{

    testx0();
    testx1();
    testx2();
    testx3();
    testxKVP();
    testxKVP2();
    testLexComment();
    testLexComment2();
    testLexGlobalWithData();
    testLexTwoRegions();
    testLexTwoKeys();
    testLexTwoRegionsWithKeys();
    testLexMangledId();
    testLex4();
    testLex5();


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
}