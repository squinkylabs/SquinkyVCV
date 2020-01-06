#pragma once

//#include "Widget.hpp"

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
    void onSelectKey(const event::SelectKey &e) override;

private:
    FramebufferWidget * fw = nullptr;
    S4ButtonDrawer * drawer = nullptr;
    callback clickHandler = nullptr;
    PasteHandler pasteHandler = nullptr;
    bool isDragging = false;

    bool handleKey(int key, int mods, int action);
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

inline bool S4Button::handleKey(int key, int mods, int action)
{
    bool handled = false;

   // DEBUG("key = %d mode= %x action = %d", key, mods, action);
   // DEBUG(" v = %d ctrl = %x press = %d", GLFW_KEY_V, RACK_MOD_CTRL, GLFW_PRESS);
    if ((key == GLFW_KEY_V) && 
    (mods & RACK_MOD_CTRL) &&
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

