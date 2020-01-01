#pragma once

#include "SqUI.h"
#include "rack.hpp"


/**
 * UI Widget that does:
 *  functions as a parameter
 *  pops up a menu of discrete choices.
 *  displays the current choice
 */

class PopupMenuParamWidget : public ::rack::app::ParamWidget
{
public:
    std::vector<std::string> labels;
    std::string text = {"pop widget default"};

    void setLabels(std::vector<std::string> l) {
        labels = l;
        ::rack::event::Change e;
        onChange(e);
    }

    void draw(const DrawArgs &arg) override;
    void onButton(const ::rack::event::Button &e) override;
    void onChange(const ::rack::event::Change &e) override;
    void onAction(const ::rack::event::Action &e) override;
};


inline void PopupMenuParamWidget::onChange(const ::rack::event::Change& e) 
 {
     if (!this->paramQuantity) {
         return;            // no module
     }

    // process ourself to update the text label
    const int index = (int) std::round( this->paramQuantity->getValue());
    if (!labels.empty()) {

        if (index < 0 || index >= (int) labels.size()) {
            fprintf(stderr, "index is outside label ranges %d\n", index);
            return;
        }
        this->text = labels[index];
    }

    // Delegate to base class to change param value
    ParamWidget::onChange(e);
}

inline void PopupMenuParamWidget::draw(const DrawArgs &args)
{
    BNDwidgetState state = BND_DEFAULT;
	bndChoiceButton(args.vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE, state, -1, text.c_str());
}

inline void PopupMenuParamWidget::onButton(const ::rack::event::Button &e)
{
     if ((e.button == GLFW_MOUSE_BUTTON_LEFT) && (e.action == GLFW_PRESS)) {
        ::rack::event::Action ea; 
        onAction(ea);
        sq::consumeEvent(&e, this);
     }
}

class PopupMenuItem : public ::rack::ui::MenuItem
{
public:
    /**
     * param index is the menu item index, but also the
     *  parameter value.
     */
    PopupMenuItem(int index, PopupMenuParamWidget * inParent) :
        index(index), parent(inParent)
    {
        // TODO: just pass text in
        text = parent->labels[index];
    }

    const int index;
    PopupMenuParamWidget* const parent;

    void onAction(const ::rack::event::Action &e) override
    {
        parent->text = this->text;
        ::rack::event::Change ce;
        // DEBUG("popup onAction, parent = %p, paramq = %p", parent, parent->paramQuantity);
        if (parent->paramQuantity) {
            parent->paramQuantity->setValue(index);
        }   
        parent->onChange(ce);
    }
};

inline void PopupMenuParamWidget::onAction(const ::rack::event::Action &e) 
{
    ::rack::ui::Menu* menu = ::rack::createMenu();

    // is choice button the right base class?
    menu->box.pos = getAbsoluteOffset(::rack::math::Vec(0, this->box.size.y)).round();
    menu->box.size.x = box.size.x;
    {
        for (int i = 0; i< (int) labels.size(); ++i) {
            menu->addChild(new PopupMenuItem(i, this));
        }
    }
}
