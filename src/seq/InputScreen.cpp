#include "InputControls.h"
#include "InputScreen.h"
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
    }

    void onDragEnd(const ::rack::event::DragEnd& e) override {
        Button::onDragEnd(e);
        //DEBUG("on DRAG END FOR ME handler = %d", bool(handler));
#if 1
        // this is my work-around for now
        if (handler) {
            DEBUG("calling handler (ourter dismisser");
            handler();
        }
#endif
    }

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
    ok->setPosition( Vec(100, 100));
    ok->setSize(Vec(80, 30));
    this->addChild(ok);   
    ok->handler = dismisser;

#if 0
    auto pop = new InputPopupMenuParamWidget();
    pop->setLabels( {"first", "second", "third"});
    pop->box.size.x = 76;    // width
    pop->box.size.y = 22;     // should set auto like button does
    pop->setPosition(Vec(100, 50));
    pop->text = "first";
    this->addChild(pop);
    inputControls.push_back(pop);
#endif
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

void InputScreen::addPitchInput(const ::rack::math::Vec& pos, const std::string& label)
{
    DEBUG("add pitch input");
    float x= pos.x;
    float y = pos.y;
    addLabel(Vec(x, y), "Axis", TEXT_COLOR );
 //   y += 30;
   // addPitchInput(Vec(x, y), "Axis");

    auto pop = new InputPopupMenuParamWidget();
    pop->setLabels( {"first", "second", "third"});
    pop->box.size.x = 76;    // width
    pop->box.size.y = 22;     // should set auto like button does
    pop->setPosition(Vec(100, 50));
    pop->text = "first";
    this->addChild(pop);
    inputControls.push_back(pop);
    DEBUG("done add pitch input");
}
