#pragma once

//#include "Widget.hpp"
#include "rack.hpp"
#include "SqGfx.h"
#include "UIPrefs.h"

class S4ButtonDrawer : public ::rack::OpaqueWidget
{
public:
    S4ButtonDrawer(const Vec& size, const Vec& pos)
    {
        this->box.size=size;
    }
    void draw(const DrawArgs &args) override;

};


/**
 * A special purpose button for the 4x4 seq module.
 * Has simple click handling, but lots of dedicated drawing ability
 */
inline void S4ButtonDrawer::draw(const DrawArgs &args)
{
    SqGfx::filledRect(
                args.vg,
                UIPrefs::NOTE_COLOR,
                this->box.pos.x, box.pos.y, box.size.x, box.size.y); 
                //x, y, width, noteHeight);
}

#define _PASTE

class S4Button : public ::rack::OpaqueWidget
{
public:
    S4Button(const Vec& size, const Vec& pos);

        /**
     * pass callback here to handle clicking on LED
     */
    using callback = std::function<void(bool isCtrlKey)>;
    void setClickHandler(callback);
    using PasteHandler = std::function<void(void)>;
    void setPasteHandler(PasteHandler);

    void onButton(const event::Button &e) override;
    void onDragHover(const event::DragHover &e) override;
    void onDragEnter(const event::DragEnter &e) override;
    void onDragLeave(const event::DragLeave &e) override;
#ifdef _PASTE
    void onSelectKey(const event::SelectKey &e) override;
#endif

private:
    FramebufferWidget * fw = nullptr;
    S4ButtonDrawer * drawer = nullptr;
    callback clickHandler = nullptr;
    PasteHandler pasteHandler = nullptr;
    bool isDragging = false;
#ifdef _PASTE
    bool handleKey(int key, int mods, int action);
#endif
};

inline S4Button::S4Button(const Vec& size, const Vec& pos)
{
    this->box.size = size;
    this->box.pos = pos;
    fw = new FramebufferWidget();
    this->addChild(fw);

    drawer = new S4ButtonDrawer(size, pos);
    fw->addChild(drawer);
}

#ifdef _PASTE
inline bool S4Button::handleKey(int key, int mods, int action)
{
    bool handled = false;

   // DEBUG("key = %d mode= %x action = %d", key, mods, action);
   // DEBUG(" v = %d ctrl = %x press = %d", GLFW_KEY_V, RACK_MOD_CTRL, GLFW_PRESS);
    
    // make v (not ctrl-v) to paste
    // can't use ctrl-v becuase rack steals it (for now)
    
    if ((key == GLFW_KEY_V) && 
    (!(mods & RACK_MOD_CTRL)) &&
    (action == GLFW_PRESS)) {
        handled = true;
        if (pasteHandler) {
            DEBUG("calling paste handler");
            pasteHandler();
        }
    }
    return handled;
}


inline void S4Button::onSelectKey(const event::SelectKey &e)
{
    bool handled = handleKey(e.key, e.mods, e.action);
    if (handled) {
        e.consume(this);
    } else {
        OpaqueWidget::onSelectKey(e);
    }
}
#endif

inline void S4Button::setClickHandler(callback h)
{
    clickHandler = h;
}

inline void S4Button::setPasteHandler(PasteHandler h)
{
    pasteHandler = h;
}

inline void S4Button::onDragHover(const event::DragHover &e)
{
    sq::consumeEvent(&e, this);
}

inline void S4Button::onDragEnter(const event::DragEnter &e)
{
}

inline void S4Button::onDragLeave(const event::DragLeave &e) 
{
    isDragging = false;
}

inline void S4Button::onButton(const event::Button &e)
{
    //printf("on button %d (l=%d r=%d)\n", e.button, GLFW_MOUSE_BUTTON_LEFT, GLFW_MOUSE_BUTTON_RIGHT); fflush(stdout);
    if ((e.button == GLFW_MOUSE_BUTTON_LEFT) && (e.action == GLFW_PRESS)) {
        // Do we need to consume this key to get dragLeave?
        isDragging = true;
        sq::consumeEvent(&e, this);
        return;
    }

    if ((e.button == GLFW_MOUSE_BUTTON_LEFT) && (e.action == GLFW_RELEASE)) {
        // Command on mac.
        const bool ctrlKey = (e.mods & RACK_MOD_CTRL);

        if (!isDragging) {
           // printf("got up when not dragging. will ignore\n"); fflush(stdout);
            return;
        }

        // OK, process it
        sq::consumeEvent(&e, this);

        if (clickHandler) {
            clickHandler(ctrlKey);
        }
    }
}

/***************************************************************************
 * 
 * S4ButtonGrid
 * 
 * 
 * bridge between the widget and the buttons
 * 
 ****************************************************************************/
using Comp = Seq4<WidgetComposite>;
//#include "app/ModuleWidget.hpp"


class S4ButtonGrid
{
public:
    void init(ModuleWidget* widget, Module* module);
private:
    std::function<void(bool isCtrlKey)> makeButtonHandler(int row, int column);
    std::function<void()> makePasteHandler(int row, int column);
};

inline void S4ButtonGrid::init(ModuleWidget* parent, Module* module)
{
    const float buttonSize = 50;
    const float buttonMargin = 10;
    const float jacksX = 380;
    for (int row = 0; row < MidiSong4::numTracks; ++row) {
        const float y = 70 + row * (buttonSize + buttonMargin);
        for (int col = 0; col < MidiSong4::numTracks; ++col) {
            const float x = 130 + col * (buttonSize + buttonMargin);
            S4Button* b = new S4Button(Vec(buttonSize, buttonSize), Vec(x, y));
            parent->addChild(b);
            b->setClickHandler(makeButtonHandler(row, col));
            b->setPasteHandler(makePasteHandler(row, col));
        }

        DEBUG("y = %.2f", y);

        const float jacksY = y + 8;
        const float jacksDy = 28;
        
        parent->addOutput(createOutputCentered<PJ301MPort>(
            Vec(jacksX, jacksY),
            module,
            Comp::CV0_OUTPUT + row));
        parent->addOutput(createOutputCentered<PJ301MPort>(
            Vec(jacksX, jacksY + jacksDy),
            module,
            Comp::GATE0_OUTPUT + row));
    }
}

inline std::function<void(bool isCtrlKey)> S4ButtonGrid::makeButtonHandler(int row, int col)
{
    return [row, col, this](bool isCtrl) {
        DEBUG("NIMP click handled, r=%d c=%d", row, col);
    };
}

inline std::function<void()> S4ButtonGrid::makePasteHandler(int row, int col)
{
    return [row, col, this]() {
        DEBUG("MINP paste handled, r=%d c=%d", row, col);
    };
}

