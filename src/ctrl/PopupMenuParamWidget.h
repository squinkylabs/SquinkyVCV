#pragma once

/**
 * UI Widget that does:
 *  functions as a parameter
 *  pops up a menu of discrete choices.
 *  displays the current choice
 */
class PopupMenuParamWidget : public ParamWidget, public ChoiceButton
{
public:
    PopupMenuParamWidget() {
        printf("in default ctor mod = %p width = %f\n",
            module, this->box.size.x );
    }
    PopupMenuParamWidget(
        Module* module,
        int paramId,
        const Vec& pos,
        float width,
        std::vector<std::string> labels);

    void test() {
        printf("here I am, in test width = %f mod=%p\n",
            this->box.size.x, module ); 
        fflush(stdout);
    }

    // override on change to init my text?

    void onAction(EventAction &e) override;
    std::vector<std::string> labels;

    // overrides for base classes
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
        EventChange ce;
        parent->setValue(index);
        parent->onChange(ce);
    }
};


inline  void PopupMenuParamWidget::onAction(EventAction &e)
{
    printf("on action\n");
    Menu* menu = gScene->createMenu();

    menu->box.pos = getAbsoluteOffset(Vec(0, this->box.size.y)).round();
    menu->box.size.x = box.size.x;
    {
        for (int i = 0; i< (int) labels.size(); ++i) {
            menu->addChild(new PopupMenuItem(i, this));
        }
    }
}

inline PopupMenuParamWidget::PopupMenuParamWidget(
    Module* module,
    int paramId, 
    const Vec& pos, 
    float width,
    std::vector<std::string> l)
{
    printf("In CTOR of PopupMenuParamWidget\n"); fflush(stdout);
    this->text = "fix me";
    this->box.pos = pos;
    this->box.size.x = width;
    this->labels = l;
  //  this->box.size.y = 50;          // just for debugging;
    this->module = module;
    this->paramId = paramId;
}