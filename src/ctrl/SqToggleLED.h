#pragma once

#include "nanovg.h"


/**
 * SqToggleLED is a light that can also give callback when it's clicked.
 * Should probably use something more standard in the future...
 */
class SqToggleLED : public ModuleLightWidget
{
public:
    SqToggleLED() {
        baseColors.resize(1);
        NVGcolor cx = nvgRGBAf(1, 1, 1, 1);
        baseColors[0] =  cx;
        this->box.size.x = 0;
        this->box.size.y = 0;
    }

    /** 
     * path is relative to our plugin,
     * unless pathIsAbsolute
     */
    void addSvg(const char* resourcePath, bool pathIsAbsolute = false);

    /**
     * pass callback here to handle clicking on LED
     */
    using callback = std::function<void(bool isCtrlKey)>;
    void setHandler(callback);

    void onButton(const event::Button &e) override;
    void onDragHover(const event::DragHover &e) override;
    void onDragEnter(const event::DragEnter &e) override;
    void onDragLeave(const event::DragLeave &e) override;
    void draw(const DrawArgs &args) override;


private:
    float getValue();
    using SvgPtr = std::shared_ptr<SqHelper::SvgWidget>;
    std::vector<SvgPtr> svgs;
    callback handler = nullptr;
    int getSvgIndex();

    bool isDragging = false;
};

inline void SqToggleLED::setHandler(callback h)
{
    handler = h;
}

inline void SqToggleLED::addSvg(const char* resourcePath, bool pathIsAbsolute)
{
    auto svg = std::make_shared<SqHelper::SvgWidget>();
    SqHelper::setSvg(svg.get(), SqHelper::loadSvg(resourcePath, pathIsAbsolute));
    svgs.push_back(svg);
    this->box.size.x = std::max(this->box.size.x, svg->box.size.x);
    this->box.size.y = std::max(this->box.size.y, svg->box.size.y);
}

inline float SqToggleLED::getValue()
{
    return color.a;
}

inline int SqToggleLED::getSvgIndex()
{
    const float _value = this->getValue();
    int index = _value > .5f ? 1 : 0;
    return index;
}


inline void SqToggleLED::draw(const DrawArgs &args)
{
    int index = getSvgIndex();
    auto svg = svgs[index];
    svg->draw(args);
}


inline void SqToggleLED::onDragHover(const event::DragHover &e)
{
   // printf("consuming drag hover\n");  fflush(stdout);
    sq::consumeEvent(&e, this);
}

inline void SqToggleLED::onDragEnter(const event::DragEnter &e)
{
    //printf("drag enter \n");  fflush(stdout);
}

inline void SqToggleLED::onDragLeave(const event::DragLeave &e) 
{
    //printf("got a drag leave, so clearing state\n"); fflush(stdout);
    isDragging = false;
}

inline void SqToggleLED::onButton(const event::Button &e)
{
    //printf("on button %d (l=%d r=%d)\n", e.button, GLFW_MOUSE_BUTTON_LEFT, GLFW_MOUSE_BUTTON_RIGHT); fflush(stdout);
    if ((e.button == GLFW_MOUSE_BUTTON_LEFT) && (e.action == GLFW_PRESS)) {
        // Do we need to consume this key to get dragLeave?
        isDragging = true;
        sq::consumeEvent(&e, this);
       //  printf("on button down\n"); fflush(stdout);
        return;
    }

    if ((e.button == GLFW_MOUSE_BUTTON_LEFT) && (e.action == GLFW_RELEASE)) {
 //printf("on button up\n"); fflush(stdout);
        const bool ctrlKey = (e.mods & GLFW_MOD_CONTROL);


        int index = getSvgIndex();
        const Vec pos(e.pos.x, e.pos.y);

        // ignore it if it's not in bounds
        if (!SqHelper::contains(svgs[index]->box, pos)) {
             //printf("on button up not contained\n"); fflush(stdout);
            return;
        }

        if (!isDragging) {
           // printf("got up when not dragging. will ignore\n"); fflush(stdout);
            return;
        }

        // OK, process it
        sq::consumeEvent(&e, this);

        if (handler) {
            handler(ctrlKey);
        }
    }
}

