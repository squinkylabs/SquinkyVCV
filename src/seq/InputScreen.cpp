#include "InputControls.h"
#include "InputScreen.h"
#include "PitchUtils.h"
#include "SqGfx.h"
#include "UIPrefs.h"

#include "../ctrl/ToggleButton.h"


using Vec = ::rack::math::Vec;
//using Button = ::rack::ui::Button;
using Widget = ::rack::widget::Widget;
using Label = ::rack::ui::Label;

InputScreen::InputScreen(const ::rack::math::Vec& pos,
    const ::rack::math::Vec& size,
    MidiSequencerPtr seq,
    const std::string& title,
    std::function<void(bool)> _dismisser) :
        sequencer(seq)
{
    box.pos = pos;
    box.size = size;
    this->dismisser = _dismisser; 
    if (!title.empty()) {
        this->addTitle(title);
    }
    addOkCancel();
}

InputScreen::~InputScreen()
{
}

std::vector<float> InputScreen::getValues() const
{
    std::vector<float> ret;
    for (auto control : inputControls) {
        ret.push_back(control->getValue());
    }
    return ret;
}

float InputScreen::getValue(int index) const
{
    return inputControls[index]->getValue();
}

bool InputScreen::getValueBool(int index) const
{
    return getValue(index) > .5 ? true : false;
}

float InputScreen::getAbsPitchFromInput(int index)
{
    assert(inputControls.size() > unsigned(index + 1));
    int iOctave = int( std::round(inputControls[index]->getValue()));
    int iSemi = int( std::round(inputControls[index+1]->getValue()));

    return PitchUtils::pitchToCV(iOctave, iSemi);
}

float InputScreen::getTransposeAmount(int index)
{
    assert(inputControls.size() > unsigned(index + 1));
    printf("getTransposeAmount nimp\n");
   // int iOctave = int( std::round(inputControls[index]->getValue()));
  //  int iSemi = int( std::round(inputControls[index+1]->getValue()));

  //  return PitchUtils::pitchToCV(iOctave, iSemi);
    return 0;
}

std::pair<int, DiatonicUtils::Modes> InputScreen::getKeysig(int index)
{
    assert(inputControls.size() > unsigned(index + 1));
    printf("getTransposeAmount nimp\n");
    return std::make_pair<int, DiatonicUtils::Modes>(0,DiatonicUtils::Modes::Major);
    
}

void InputScreen::draw(const Widget::DrawArgs &args)
{
    NVGcontext *vg = args.vg;
    SqGfx::filledRect(vg, UIPrefs::NOTE_EDIT_BACKGROUND, 0, 0, box.size.x, box.size.y);
    Widget::draw(args);
}

// TODO: rename or move this style info
const NVGcolor TEXT_COLOR = nvgRGB(0xc0, 0xc0, 0xc0);

Label* InputScreen::addLabel(const Vec& v, const char* str, const NVGcolor& color = TEXT_COLOR)
{
    Label* label = new Label();
    label->box.pos = v;
    label->text = str;
    label->color = color;
    this->addChild(label);
    return label;
}

void InputScreen::addTitle(const std::string& title)
{
    const float x = 0;
    const float y = 20;
    std::string titleText = "** " + title + " **";
    auto l = addLabel(Vec(x, y), titleText.c_str(),  TEXT_COLOR);
    l->box.size.x = this->box.size.x;
    l->alignment = Label::CENTER_ALIGNMENT;
}

static std::vector<std::string> octaves = {
    "-3", "-2", "-1", "0", "1", "2", "3", "4", "5", "6", "7"
};

static std::vector<std::string> semis = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

void InputScreen::addPitchInput(const ::rack::math::Vec& pos, const std::string& label)
{
    float x= 0;
    float y = pos.y;
    auto l = addLabel(Vec(x, y), label.c_str(), TEXT_COLOR );
    l->box.size.x = pos.x - 10;
    l->alignment = Label::RIGHT_ALIGNMENT;
  
    x = pos.x;

    auto pop = new InputPopupMenuParamWidget();
    pop->setLabels( octaves);
    pop->box.size.x = 76;    // width
    pop->box.size.y = 22;     // should set auto like button does
    pop->setPosition(Vec(x, y));
    pop->text = "0";
    this->addChild(pop);
    inputControls.push_back(pop);

    x += 80;
    pop = new InputPopupMenuParamWidget();
    pop->setLabels( semis);
    pop->box.size.x = 76;    // width
    pop->box.size.y = 22;     // should set auto like button does
    pop->setPosition(Vec(x, y));
    pop->text = "C";
    this->addChild(pop);
    inputControls.push_back(pop);
}

static std::vector<std::string> octavesRel = {
    "-7 oct", "-6 oct", "-5 oct",
    "-4 oct", "-3 oct", "-2 oct", "-1 oct",
    "+0 oct",
    "+1 oct", "+2 oct", "+3 oct", "+4 oct",
    "+5 oct", "+6 oct", "+7 oct"
};

static std::vector<std::string> semisRel = {
    "-12 semi", "-11 semi", "-10 semi","-9 semi"
    "-8 semi", "-7 semi", "-6 semi","-5 semi"
    "-4 semi", "-3 semi", "-2 semi","-1 semi",
     "-", 
     "+1 semi", "+2 semi", "+3 semi","+4 semi",
     "+5 semi", "+6 semi", "+7 semi","+8 semi",
     "+9 semi","+10 semi","+11 semi", "+12 semi"
};

void InputScreen::addPitchOffsetInput(const ::rack::math::Vec& pos, const std::string& label)
{
    float x= 0;
    float y = pos.y;
    auto l = addLabel(Vec(x, y), label.c_str(), TEXT_COLOR );
    l->box.size.x = pos.x - 10;
    l->alignment = Label::RIGHT_ALIGNMENT;
  
    x = pos.x;

    auto pop = new InputPopupMenuParamWidget();
    pop->setLabels( octavesRel);
    pop->box.size.x = 76;    // width
    pop->box.size.y = 22;     // should set auto like button does
    pop->setPosition(Vec(x, y));
    pop->text = "+0 oct";
    this->addChild(pop);
    inputControls.push_back(pop);

    x += 80;
    pop = new InputPopupMenuParamWidget();
    pop->setLabels( semisRel);
    pop->box.size.x = 76;    // width
    pop->box.size.y = 22;     // should set auto like button does
    pop->setPosition(Vec(x, y));
    pop->text = "-";
    this->addChild(pop);
    inputControls.push_back(pop);
}

static std::vector<std::string> roots = {
    "C", "C#", "D", "D#",
    "E", "F", "F#", "G",
    "G#",  "A", "A#", "B"
};

static std::vector<std::string> modes = {
    "Major", "Dorian", "Phrygian", "Lydian",
    "Mixolydian", "Minor", "Locrian"
};

void InputScreen::addKeysigInput(const ::rack::math::Vec& pos)
{
    float x= 0;
    float y = pos.y;
    auto l = addLabel(Vec(x, y), "Key Signature", TEXT_COLOR );
    l->box.size.x = pos.x - 10;
    l->alignment = Label::RIGHT_ALIGNMENT;
  
    x = pos.x;

    auto pop = new InputPopupMenuParamWidget();
    pop->setLabels( roots);
    pop->box.size.x = 76;    // width
    pop->box.size.y = 22;     // should set auto like button does
    pop->setPosition(Vec(x, y));
    pop->text = "C";
    this->addChild(pop);
    inputControls.push_back(pop);

    x += 80;
    pop = new InputPopupMenuParamWidget();
    pop->setLabels( modes);
    pop->box.size.x = 76;    // width
    pop->box.size.y = 22;     // should set auto like button does
    pop->setPosition(Vec(x, y));
    pop->text = "Major";
    this->addChild(pop);
    inputControls.push_back(pop);  
}


void InputScreen::addConstrainToScale(const ::rack::math::Vec& pos)
{
    auto check = new CheckBox();
    check->box.pos = pos;
    check->box.size = Vec(17, 17);
    this->addChild(check);
    inputControls.push_back(check);

    // rationalize this
    const NVGcolor TEXT_COLOR = nvgRGB(0xc0, 0xc0, 0xc0);

    auto l = addLabel(Vec(0, pos.y), "Constrain to scale", TEXT_COLOR );
    l->box.size.x = centerColumn - centerGutter;;
    l->alignment = Label::RIGHT_ALIGNMENT;
}

void InputScreen::addOkCancel()
{
    auto ok = new Button2();
    const float y = okCancelY;
    ok->text = "OK";
    float x = 60;

    ok->setPosition( Vec(x, y));
    ok->setSize(Vec(80, 22));
    this->addChild(ok);   
    ok->handler = [this]() {
        dismisser(true);
    };

    auto cancel = new Button2();
    cancel->handler = [this]() {
        dismisser(false);
    };
    cancel->text = "Cancel";
    x = 250;
    cancel->setPosition( Vec(x, y));
    cancel->setSize(Vec(80, 22));
    this->addChild(cancel);   
}
