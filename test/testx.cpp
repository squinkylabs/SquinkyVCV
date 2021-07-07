

#include <set>

#include "FilePath.h"
#include "RandomRange.h"
#include "SInstrument.h"
#include "SParse.h"
#include "SqLog.h"
#include "asserts.h"

/**
 * Naming conventions for SFZ tests.
 * mostly in testx:

 * testParse... - general parsing test.

 * Mostly in testxLex:
 * testLex... - any test of the lexer only.
 * 
 * mostly in testx2.cpp
 * testWaveLoader... - test of the save loader
 * testParseHeading...  specific parse tests around headings.
 * testCompiledRegion... mostly tests of the lower level compiled region struct
 * testCompile.... and test that goes all the way to a compiled instrument
 * 
 * mostly in testx3
 * testRegion...    test the CompiledRegion object itself
 * testPlay..   compiles an instrument and plays it
 * 
 * mostly in testx4
 * testFilePath.. test of the filePath object that is used a lot if SFZ Player
 * testSchema... test of the SamplerSchema class.
 * 
 * mostly in testx5
 * testSampler4v... tests of the lower level sample playback class
 */

static void testParse1() {
    SInstrumentPtr inst = std::make_shared<SInstrument>();

    auto err = SParse::go("random-text", inst);
    assert(!err.empty());
}

static void testParseRegion() {
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("<region>", inst);
    assert(err.empty());

    assertEQ(inst->headings.size(), 1);
    assertEQ(int(inst->headings[0]->type), int(SHeading::Type::Region));
}

static void testParse2() {
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("<region>pitch_keycenter=24", inst);
    assert(err.empty());

    assertEQ(inst->headings.size(), 1);

    SHeadingPtr region = inst->headings[0];
    assertEQ(int(region->type), int(SHeading::Type::Region));

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

static void testParseGroupAndValues() {
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

static void testParseComplexDrum1() {
    const char* p = R"foo(
<group> volume=-29 amp_veltrack=100 loop_mode=one_shot key=54 group=2         // crash1Choke /////
<region> sample=OH\crash1Choke_OH_F_1.wav 

<group> volume=-19 amp_veltrack=95 ampeg_release=0.6 key=55 loop_mode=one_shot lovel=1 hivel=59 off_mode=normal off_by=2		// crash1 ////5 Samples Random!
<region> sample=OH\crash1_OH_P_1.wav lorand=0 hirand=0.2
)foo";
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go(p, inst);
    assert(err.empty());
    assertEQ(inst->headings.size(), 4);

    assertEQ(int(inst->headings[1]->type), int(SHeading::Type::Region));
    auto region = inst->headings[1];
    assertEQ(region->values.size(), 1);
    std::string filePath = region->values[0]->value;
    FilePath fp(filePath);
    assertEQ(fp.getExtensionLC().size(), 3);
}

static void testParseComplexDrum() {
    const char* p = R"foo(
<region>sample=OH\crash1Choke_OH_F_1.wav 
<group>
)foo";
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go(p, inst);
    assert(err.empty());
    assertEQ(inst->headings.size(), 2);

    assertEQ(int(inst->headings[0]->type), int(SHeading::Type::Region));
    auto region = inst->headings[0];
    assertEQ(region->values.size(), 1);
    std::string filePath = region->values[0]->value;
    FilePath fp(filePath);
    assertEQ(fp.getExtensionLC().size(), 3);
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

// We aren't using these random ranges any more, but might as will keep alive.
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

static void testParseLineNumbers() {
const char* p = R"foo(
// 1
// 2
// 3
<group>
// 5
// 6
<region>
// 8
<region>
// 10
// 11
// 12
// 13
<region>
// 15
// 16
// 17
// 18
// 19
)foo";

    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go(p, inst);
    assert(err.empty());

    assertEQ(inst->headings.size(), 4);
    assertEQ(inst->headings[0]->lineNumber, 4);
    assertEQ(inst->headings[1]->lineNumber, 7);
    assertEQ(inst->headings[2]->lineNumber, 9);
    assertEQ(inst->headings[3]->lineNumber, 14);
}

extern int compileCount;

void testx() {
    assertEQ(compileCount, 0);
    assert(parseCount == 0);

    testParse1();
    testParseRegion();
    testParse2();
    testParseGlobal();

    testParseGlobalAndRegion();
    testParseGlobalGroupAndRegion();

    testParseComment();
    testParseGroups();

    testParseGroupAndValues();
    testParseLabel2();

    testParseGlobalWithData();
    testParseTwoGroupsA();
    testParseTwoGroupsB();
    testparse_piano1();

    // testparse_piano2b();
    testparse_piano2();
    testParseSimpleDrum();
    testParseComplexDrum();
    testRandomRange0();
    testRandomRange1();

    // merge conflict here. does this work? a: it was deleted in main
    //testParseDX();
    testParseCurve();
    testParseLineNumbers();
    assert(parseCount == 0);
}