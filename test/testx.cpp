

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

static void testx4()
{
    SLexPtr lex = SLex::go("abc=def");
    assert(lex);
    assertEQ(lex->items.size(), 3);
    assert(lex->items[0]->itemType == SLexItem::Type::Identifier);
    SLexIdentifier* pid = static_cast<SLexIdentifier *>(lex->items[0].get());
    assertEQ(pid->idName, "abc");
}

static void testparse1()
{
    auto inst = SParse::go("random-text");
    assert(!inst);
}

static void testparse2()
{
    printf("start testprse2\n");
    auto inst = SParse::go("<region>pitch_keycenter=24");
    assert(inst);
}

void testx()
{
    testx0();
    testx1();
    testx2();
    testx3();
    testx4();

    testparse1();
    testparse2();
}