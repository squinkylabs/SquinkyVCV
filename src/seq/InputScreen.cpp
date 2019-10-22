#include "InputScreen.h"
#include "SqGfx.h"
#include "UIPrefs.h"

#include "../ctrl/ToggleButton.h"
#include "../ctrl/PopupMenuParamWidgetv1.h"

using Vec = ::rack::math::Vec;
using Button = ::rack::ui::Button;

class Button2 : public Button
{
public:
    // this isn't firing. don't know why
    void onAction(const ::rack::event::Action& e) override {
        DEBUG("onAction from button");
    }

    void onDragEnd(const ::rack::event::DragEnd& e) override {
        Button::onDragEnd(e);
        DEBUG("on DRAG END FOR ME handler = %d", bool(handler));
        if (handler) {
            DEBUG("calling handler (ourter dismisser");
            auto tempHandler = handler;
            handler = nullptr;                  // only call it onece
            tempHandler();
        }
    }

    ~Button2() {
        DEBUG("dtor of button");
    }

    std::function<void()> handler = nullptr;
};

InputScreen::InputScreen(const ::rack::math::Vec& pos,
        const ::rack::math::Vec& size,
        std::function<void()> _dismisser)
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

    auto pop = new PopupMenuParamWidget();
    pop->setLabels( {"first", "second", "third"});
    pop->box.size.x = 76;    // width
    pop->box.size.y = 22;     // should set auto like button does
    pop->setPosition(Vec(100, 50));
    pop->text = "first";
    this->addChild(pop);
}

InputScreen::~InputScreen()
{
    DEBUG("dtor of input screen %p", this);
}

void InputScreen::draw(const Widget::DrawArgs &args)
{
    NVGcontext *vg = args.vg;
    SqGfx::filledRect(vg, UIPrefs::NOTE_EDIT_BACKGROUND, 0, 0, box.size.x, box.size.y);
    Widget::draw(args);
}

#if 0
 void InputScreen::onButton(const ::rack::event::Button &e)
 {
     Widget::onButton(e);
    DEBUG("InputScreen::onButton"); fflush(stdout);
    if (dismisser) {
        dismisser();
    } else DEBUG("no dismisser");
    e.consume(this);
 }
 #endif

//*********************************************************************




#if 0
void InputScreenSet::show(::rack::widget::Widget* parent)
{
    ::rack::widget::Widget* screen = screens[0].get();
    currentScreenIndex = 0;
    parent->addChild(screen);
    parentWidget = parent;
}

void InputScreenSet::dismiss()
{
    DEBUG("iss::dismiss");
    if (isDismissing) {
        DEBUG("leaving dismiss on re-enter\n");
        return;
    }
    DEBUG("iss::dismiss 15");
    isDismissing = true;
    ::rack::widget::Widget* screen = screens[0].get();

    DEBUG("about to clear all the children of my screen");
    screen->clearChildren();

// we really need to remove screen from parent.

    DEBUG("iss::dismiss about to remove screen from parent scree = %d parent = %d", bool(screen), bool(parentWidget));
    parentWidget->removeChild(screen);
    parentWidget = nullptr;
    currentScreenIndex = 0;
     DEBUG("iss::dismiss 3");
}

void InputScreenSet::add(InputScreenPtr is)
{
    screens.push_back(is);
}

InputScreenSet::~InputScreenSet()
{
    DEBUG("dtor iss");
    dismiss();
   DEBUG("dtor iss 2");
}
#endif