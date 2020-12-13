

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
    assertEQ(lex->items.size(), 1);
    assert(lex->items[0]->itemType == SLexItem::Type::Tag);
    SLexTag* ptag = static_cast<SLexTag *>(lex->items[0].get());
    assertEQ(ptag->tagName, "global");
}

static void testx2()
{
    SLexPtr lex = SLex::go("=");
    assert(lex);
    assertEQ(lex->items.size(), 1);
    assert(lex->items[0]->itemType == SLexItem::Type::Equal);
    //SLexTag* ptag = static_cast<SLexTag*>(lex->items[0].get());
   // assertEQ(ptag->tagName, "global");
}


static void testx3()
{
    SLexPtr lex = SLex::go("qrst");
    assert(lex);
    assertEQ(lex->items.size(), 1);
    assert(lex->items[0]->itemType == SLexItem::Type::Identifier);
    SLexIdentifier* pid = static_cast<SLexIdentifier *>(lex->items[0].get());
    assertEQ(pid->idName, "qrst");
}

static void testxKVP()
{
    SLexPtr lex = SLex::go("abc=def");
    assert(lex);
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
    assertEQ(lex->items.size(), 1);
    assert(lex->items[0]->itemType == SLexItem::Type::Tag);
    SLexTag* pTag = static_cast<SLexTag*>(lex->items[0].get());
    assertEQ(pTag->tagName, "global")
}


static void testLexComment2()
{
    SLexPtr lex = SLex::go("// comment\n//comment\n\n<global>\n\n");
    assert(lex);
    assertEQ(lex->items.size(), 1);
    assert(lex->items[0]->itemType == SLexItem::Type::Tag);
    SLexTag* pTag = static_cast<SLexTag*>(lex->items[0].get());
    assertEQ(pTag->tagName, "global")
}


static void testparse1()
{
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("random-text", inst);
    assert(err.empty());
}

static void testparse2()
{
    printf("start testprse2\n");
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("<region>pitch_keycenter=24", inst);
    assert(err.empty());;
}

static void testParseGlobal()
{
    printf("start test parse global\n");
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("<global>", inst);
    assert(err.empty());
}

static void testParseComment()
{
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("// comment\n<global>", inst);
    assert(err.empty());
}

static void testParseGlobalWithData()
{
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("<global>ampeg_release=0.6", inst);
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
    #if 1
    testx0();
    testx1();
    testx2();
    testx3();
    testxKVP();
    testxKVP2();
    testLexComment();
    testLexComment2();

    testparse1();
    testparse2();
    testParseGlobal();
    testParseComment();
    #endif
    testParseGlobalWithData();
    testparse_piano1();
}