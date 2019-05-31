#pragma once

/**
 * UI Widget that does:
 *  functions as a parameter
 *  pops up a menu of discrete choices.
 *  displays the current choice
 */
#ifndef __V1x
class PopupMenuParamWidget : virtual public ParamWidget, virtual public ChoiceButton
{
public:
    std::vector<std::string> labels;

    void setLabels(std::vector<std::string> l) {
        labels = l;
        EventChange e;
        onChange(e);
    }

    void onChange(EventChange& e) override {
        // process ourself to update the text label
        const int index = (int) std::round( this->value);
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

    void onAction(EventAction &e) override;
    

    /** overrides for base classes
     * TODO: should we call both base methods? 
     * Only the one we think does something?
     */
    void onMouseDown(EventMouseDown &e) override {
        ParamWidget::onMouseDown(e);
        ChoiceButton::onMouseDown(e);
    }
    void onMouseUp(EventMouseUp &e) override {
         ParamWidget::onMouseUp(e);
        ChoiceButton::onMouseUp(e);
    }
    void onMouseMove(EventMouseMove &e) override {
        ChoiceButton::onMouseMove(e);
    }
    void onScroll(EventScroll &e) override {
        ParamWidget::onScroll(e);
    }   
};


// better yet, don't pass any text here, we can just call back.
// And fix the menu item leaks!
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


    void onAction(EventAction &e) override
    {
        parent->text = this->text;
        EventChange ce;
        parent->setValue(index);
        parent->onChange(ce);
    }
};

inline  void PopupMenuParamWidget::onAction(EventAction &e)
{
    Menu* menu = gScene->createMenu();

    menu->box.pos = getAbsoluteOffset(Vec(0, this->box.size.y)).round();
    menu->box.size.x = box.size.x;
    {
        for (int i = 0; i< (int) labels.size(); ++i) {
            menu->addChild(new PopupMenuItem(i, this));
        }
    }
}
#endif

