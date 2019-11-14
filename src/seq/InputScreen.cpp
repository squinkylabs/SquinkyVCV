#include "InputControls.h"
#include "InputScreen.h"
#include "ISeqSettings.h"
#include "MidiSequencer.h"
#include "PitchInputWidget.h"
#include "PitchUtils.h"
#include "SqGfx.h"
#include "UIPrefs.h"

#include "../ctrl/ToggleButton.h"

using Vec = ::rack::math::Vec;
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

int InputScreen::getValueInt(int index) const
{
    return int (std::round(getValue(index)));
}

std::pair<int, Scale::Scales> InputScreen::getKeysig(int index)
{
    assert(inputControls.size() > unsigned(index + 1));

    const int iRoot = getValueInt(index);
    const int iMode = getValueInt(index+1);
    const Scale::Scales mode = Scale::Scales(iMode);
    DEBUG("get keySig = %d (root) %d (mode)", iRoot, iMode);
    return std::make_pair(iRoot, mode);
}

void InputScreen::saveKeysig(int index)
{
    auto keysig = getKeysig(index);
    if (sequencer) {
        sequencer->context->settings()->setKeysig(keysig.first, keysig.second);
    }
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

void InputScreen::addPitchInput(
    const ::rack::math::Vec& pos,
    const std::string& label,
    std::function<void(void)>callback)
{
    // new way, let's ignore the passed x value
    // todo: make caller pass correct coord
    ::rack::math::Vec pos2 = pos;
    pos2.x = 0;
   
    ::rack::math::Vec size = box.size;
    size.y = controlRow(2);
    DEBUG("add pitch offset abs, height=%.2f ok = %.2f", size.y, okCancelY);

    auto p = new PitchInputWidget(pos2, size, label, false);
    p->setCallback(callback);
    inputControls.push_back(p);
    this->addChild(p);
}

void InputScreen::addPitchOffsetInput(
    const ::rack::math::Vec& pos, 
    const std::string& label,
    std::function<void(void)>callback)
{
    // new way, let's ignore the passed x value
    // todo: make caller pass correct coord
    ::rack::math::Vec pos2 = pos;
    pos2.x = 0;

    ::rack::math::Vec size = box.size;
    size.y = controlRow(2);
    DEBUG("add pitch offset, height=%.2f ok = %.2f", size.y, okCancelY);

    auto p = new PitchInputWidget(pos2, size, label, true);
    p->setCallback(callback);
    inputControls.push_back(p);
    this->addChild(p);
}

static std::vector<std::string> roots = {
    "C", "C#", "D", "D#",
    "E", "F", "F#", "G",
    "G#",  "A", "A#", "B"
};

static std::vector<std::string> modes = {
    "Major", "Dorian", "Phrygian", "Lydian",
    "Mixolydian", "Minor", "Locrian", "Minor Pentatonic"
};

void InputScreen::addKeysigInput(const ::rack::math::Vec& pos, std::pair<int, Scale::Scales> keysig)
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
    pop->setValue(keysig.first);

    x += 80;
    pop = new InputPopupMenuParamWidget();
    pop->setLabels( modes);
    pop->box.size.x = 76;    // width
    pop->box.size.y = 22;     // should set auto like button does
    pop->setPosition(Vec(x, y));
    pop->text = "Major";
    this->addChild(pop);
    inputControls.push_back(pop);
    pop->setValue( int(keysig.second)); 
}

void InputScreen::addNumberChooserInt(const ::rack::math::Vec& pos, const char* str, int nMin, int nMax)
{
    float x= 0;
    float y = pos.y;
    auto l = addLabel(Vec(x, y), str, TEXT_COLOR );
    l->box.size.x = pos.x - 10;
    l->alignment = Label::RIGHT_ALIGNMENT;
  
    x = pos.x;

    std::vector<std::string> labels;
    for (int i= nMin; i<= nMax; ++i) {
        char buf[100];
        snprintf(buf, sizeof(buf), "%d", i);
        std::string s(buf);
        labels.push_back(s);
    }
    auto pop = new InputPopupMenuParamWidget();
    pop->setLabels( labels);
    pop->box.size.x = 76;    // width
    pop->box.size.y = 22;     // should set auto like button does
    pop->setPosition(Vec(x, y));
    pop->text = labels[0];
    pop->setValue(0);

    this->addChild(pop);
    inputControls.push_back(pop);
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
