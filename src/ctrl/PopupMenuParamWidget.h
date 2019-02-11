#pragma once

/**
 * UI Widget that does:
 *  functions as a parameter
 *  pops up a menu of discrete choices.
 *  displays the current choice
 */
class PopupMenuParamWidget : public ChoiceButton
{
public:
    PopupMenuParamWidget(
        int param,
        const Vec& pos,
        float width,
        std::vector<std::string> labels);

    void onAction(EventAction &e) override;
    std::vector<std::string> labels;
private:

      
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
       // *rangeOut = values[rangeIndex];
       printf("I need to set param value from here\n"); fflush(stdout);
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
       /* menu->addChild(new v(1, output, this));
        menu->addChild(new RangeItem(2, output, this));
        menu->addChild(new RangeItem(3, output, this));
        menu->addChild(new RangeItem(4, output, this));
        */
    }
}

inline PopupMenuParamWidget:: PopupMenuParamWidget(
    int param, 
    const Vec& pos, 
    float width,
    std::vector<std::string> l)
{
    this->text = "testing";
    this->box.pos = pos;
    this->box.size.x = width;
    this->labels = l;
  //  this->box.size.y = 50;          // just for debugging;
}