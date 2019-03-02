#pragma once

#ifdef __V1

/**
 * UI Widget that does:
 *  functions as a parameter
 *  pops up a menu of discrete choices.
 *  displays the current choice
 */

class PopupMenuParamWidget : public ParamWidget
{
public:
    std::vector<std::string> labels;
    std::string text = {"test"};

    void setLabels(std::vector<std::string> l) {
        labels = l;
        ChangeEvent e;
        onChange(e);
    }

    void draw(const DrawArgs &arg) override;
    void onButton(const ButtonEvent &e) override;
    void onChange(const ChangeEvent &e) override;
    void onAction(const ActionEvent &e) override;
};


inline void PopupMenuParamWidget::onChange(const ChangeEvent& e) 
 {
     if (!this->paramQuantity) {
         return;            // no module
     }
    // process ourself to update the text label
    const int index = (int) std::round( this->paramQuantity->getValue());
    if (!labels.empty()) {
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

inline void PopupMenuParamWidget::onButton(const ButtonEvent &e)
{
    // for now, let's activate on all mouse clicks
    ActionEvent ea; 
    onAction(ea);
}

class PopupMenuItem : public MenuItem
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


    void onAction(const ActionEvent &e) override
    {
        parent->text = this->text;
        ChangeEvent ce;
        parent->paramQuantity->setValue(index);
        parent->onChange(ce);
    }
};

void PopupMenuParamWidget::onAction(const ActionEvent &e) 
{
    Menu* menu = createMenu();

    // is choice button the right base class?
    menu->box.pos = getAbsoluteOffset(Vec(0, this->box.size.y)).round();
    menu->box.size.x = box.size.x;
    {
        for (int i = 0; i< (int) labels.size(); ++i) {
            menu->addChild(new PopupMenuItem(i, this));
        }
    }
}
#endif
