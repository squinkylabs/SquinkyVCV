#include "InputControls.h"
#include "InputScreen.h"
#include "PitchUtils.h"
#include "SqGfx.h"
#include "UIPrefs.h"

#include "../ctrl/ToggleButton.h"


using Vec = ::rack::math::Vec;
using Button = ::rack::ui::Button;
using Widget = ::rack::widget::Widget;
using Label = ::rack::ui::Label;

class Button2 : public Button
{
public:
    // this isn't firing. don't know why
    void onAction(const ::rack::event::Action& e) override {
        DEBUG("onAction from button");
        if (handler) {
            handler();
        }
    }
#if 0
    void onDragEnd(const ::rack::event::DragEnd& e) override {
        Button::onDragEnd(e);
        //DEBUG("on DRAG END FOR ME handler = %d", bool(handler));

        // this is my work-around for now
        if (handler) {
            DEBUG("calling handler (ourter dismisser");
            handler();
        }

    }
    #endif

    ~Button2() {
        DEBUG("dtor of button");
    }

    std::function<void()> handler = nullptr;
};

InputScreen::InputScreen(const ::rack::math::Vec& pos,
    const ::rack::math::Vec& size,
    MidiSequencerPtr seq,
    std::function<void()> _dismisser) :
        sequencer(seq)
{
    box.pos = pos;
    box.size = size;
    this->dismisser = _dismisser; 
    DEBUG("dismisser = %d", bool(_dismisser));

    auto ok = new Button2();
    ok->text = "OK";
    float x = 100;
    float y = 200;
    ok->setPosition( Vec(x, y));
    ok->setSize(Vec(80, 30));
    this->addChild(ok);   
    ok->handler = dismisser;

}


InputScreen::~InputScreen()
{
    DEBUG("dtor of input screen %p", this);
}

std::vector<float> InputScreen::getValues() const
{
    std::vector<float> ret;
    for (auto control : inputControls) {
        ret.push_back(control->getValue());
    }
    return ret;
}

void InputScreen::draw(const Widget::DrawArgs &args)
{
    NVGcontext *vg = args.vg;
    SqGfx::filledRect(vg, UIPrefs::NOTE_EDIT_BACKGROUND, 0, 0, box.size.x, box.size.y);
    Widget::draw(args);
}

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

static std::vector<std::string> octaves = {
    "-3", "-2", "-1", "0", "1", "2", "3", "4", "5", "6", "7"
};

static std::vector<std::string> semis = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G"
};

void InputScreen::addPitchInput(const ::rack::math::Vec& pos, const std::string& label)
{
    DEBUG("add pitch input");
    float x= pos.x;
    float y = pos.y;
    addLabel(Vec(x, y), "Axis", TEXT_COLOR );
    y += 30;


    auto pop = new InputPopupMenuParamWidget();
    pop->setLabels( octaves);
    pop->box.size.x = 76;    // width
    pop->box.size.y = 22;     // should set auto like button does
    pop->setPosition(Vec(x, y));
    pop->text = "0";
    this->addChild(pop);
    inputControls.push_back(pop);
    DEBUG("done add pitch input");

    y += 40;
    pop = new InputPopupMenuParamWidget();
    pop->setLabels( semis);
    pop->box.size.x = 76;    // width
    pop->box.size.y = 22;     // should set auto like button does
    pop->setPosition(Vec(x, y));
    pop->text = "C";
    this->addChild(pop);
    inputControls.push_back(pop);
}

float InputScreen::getAbsPitchFromInput(int index)
{
    assert(inputControls.size() > unsigned(index + 1));
    int iOctave = int( std::round(inputControls[index]->getValue()));
    int iSemi = int( std::round(inputControls[index+1]->getValue()));

    return PitchUtils::pitchToCV(iOctave, iSemi);
}
